// registry.cpp: Win32 registry utilities

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM2_SOURCE
#define NOMINMAX
#include <hamigaki/bjam2/util/win32/registry.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>
#include <locale>
#include <sstream>
#include <windows.h>

namespace hamigaki { namespace bjam2 { namespace win32 {

namespace
{

::HKEY root_key(const std::string& s)
{
    if (s == "HKLM")
        return HKEY_LOCAL_MACHINE;
    else if (s == "HKCU")
        return HKEY_CURRENT_USER;
    else if (s == "HKCR")
        return HKEY_CLASSES_ROOT;
    else if (s == "HKEY_LOCAL_MACHINE")
        return HKEY_LOCAL_MACHINE;
    else if (s == "HKEY_CURRENT_USER")
        return HKEY_CURRENT_USER;
    else if (s == "HKEY_CLASSES_ROOT")
        return HKEY_CLASSES_ROOT;
    else
        return 0;
}

string_list expand_env(const char* s)
{
    ::DWORD size = 256;
    boost::scoped_array<char> buf(new char[size]);

    ::DWORD res;
    while (
        res = ::ExpandEnvironmentStringsA(s, buf.get(), size),
        res > size
    )
    {
        size = res;
        buf.reset(new char[size]);
    }

    if (res == 0)
        return string_list();
    else
        return string_list(std::string(buf.get(), res-1));
}

class registry_key : boost::noncopyable
{
public:
    registry_key(
        ::HKEY parent, const std::string& sub_key, ::REGSAM sam)
    {
        long err = ::RegOpenKeyExA(
            parent, sub_key.c_str(), 0, sam, &handle_);
        if (err != ERROR_SUCCESS)
            handle_ = 0;
    }

    ~registry_key()
    {
        if (handle_ != 0)
            ::RegCloseKey(handle_);
    }

    string_list get_values(const boost::optional<std::string>& name) const
    {
        if (handle_ == 0)
            return string_list();

        const char* name_s = 0;
        if (name)
            name_s = name->c_str();

        ::DWORD type;
        ::DWORD size = 0;
        long err = ::RegQueryValueExA(handle_, name_s, 0, &type, 0, &size);
        if (err != ERROR_SUCCESS)
            return string_list();

        // "+2" for the broken string
        boost::scoped_array<char> buf(new char[size+2]);

        err = ::RegQueryValueExA(handle_, name_s, 0,
            &type, reinterpret_cast< ::BYTE*>(buf.get()), &size);
        if (err != ERROR_SUCCESS)
            return string_list();

        if (type == REG_EXPAND_SZ)
        {
            if (size == 0)
                return string_list();

            if (buf[size-1] != '\0')
                buf[size++] = '\0';

            return expand_env(buf.get());
        }
        else if (type == REG_MULTI_SZ)
        {
            if (size < 2)
                return string_list();

            if (buf[size-1] != '\0')
                buf[size++] = '\0';
            if (buf[size-2] != '\0')
                buf[size++] = '\0';

            string_list result;
            for (const char* s = buf.get(); *s; )
            {
                std::string tmp(s);
                result += tmp;
                s += (tmp.size() + 1);
            }
            return result;
        }
        else if (type == REG_DWORD)
        {
            if (size != 4)
                return string_list();

            std::ostringstream os;
            os.imbue(std::locale::classic());
            os << *reinterpret_cast< ::DWORD*>(buf.get());
            return string_list(os.str());
        }
        else if (type == REG_SZ)
        {
            if (size == 0)
                return string_list();

            if (buf[size-1] == '\0')
                --size;

            return string_list(std::string(buf.get(), size));
        }
        else
            return string_list();
    }

private:
    ::HKEY handle_;
};

} // namespace

HAMIGAKI_BJAM2_DECL
string_list registry_values(
    const std::string& key, const boost::optional<std::string>& name)
{
    std::string::size_type delim = key.find("\\");

    ::HKEY parent = root_key(key.substr(0, delim));
    if (parent == 0)
        return string_list();

    std::string::size_type start = delim;
    if (start != std::string::npos)
        ++start;

    registry_key reg(parent, key.substr(start), KEY_READ);
    return reg.get_values(name);
}

} } } // End namespaces win32, bjam2, hamigaki.
