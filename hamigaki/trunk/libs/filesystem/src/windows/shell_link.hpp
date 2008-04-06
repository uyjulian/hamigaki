// shell_link.hpp: the shell link operations

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_WINDOWS_SHELL_LINK_HPP
#define HAMIGAKI_FILESYSTEM_WINDOWS_SHELL_LINK_HPP

#include <boost/scoped_array.hpp>
#include <stdexcept>
#include <windows.h>
#include <objbase.h>
#include <objidl.h>
#include <shlobj.h>

namespace hamigaki { namespace filesystem { namespace detail {

class shell_error
{
public:
    explicit shell_error(::HRESULT res) : res_(res)
    {
    }

    HRESULT result() const
    {
        return res_;
    }

private:
    HRESULT res_;
};

inline void shell_check(HRESULT res)
{
    if (FAILED(res))
        throw shell_error(res);
}


class persist_file
{
public:
    explicit persist_file(IUnknown* p)
    {
        // IID_IPersistFile
        const ::GUID iid =
        {
            0x0000010B, 0x0000, 0x0000,
            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
        };

        void* tmp;
        shell_check(p->QueryInterface(iid, &tmp));
        ptr_ = static_cast<IPersistFile*>(tmp);
    }

    ~persist_file()
    {
        ptr_->Release();
    }

    void save(const wchar_t* ph)
    {
        ptr_->Save(ph, TRUE);
    }

    void save(const char* ph)
    {
        int buf_size = ::MultiByteToWideChar(CP_ACP, 0, ph, -1, 0, 0);
        if (buf_size == 0)
            throw std::runtime_error("invalid path name");

        boost::scoped_array<wchar_t> buf(new wchar_t[buf_size]);
        buf_size =
            ::MultiByteToWideChar(CP_ACP, 0, ph, -1, buf.get(), buf_size);
        if (buf_size == 0)
            throw std::runtime_error("invalid path name");

        ptr_->Save(buf.get(), TRUE);
    }

private:
    IPersistFile* ptr_;

    persist_file(const persist_file&);
    persist_file& operator=(const persist_file&);
};


template<class Char>
struct shell_link_traits;

template<>
struct shell_link_traits<char>
{
    typedef IShellLinkA interface_type;

    static IShellLinkA* create()
    {
        // CLSID_ShellLink
        const GUID clsid =
        {
            0x00021401, 0x0000, 0x0000,
            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
        };

        // IID_IShellLinkA
        const GUID iid =
        {
            0x000214EE, 0x0000, 0x0000,
            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
        };

        void* p;
        shell_check(
            ::CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, iid, &p)
        );

        return static_cast<IShellLinkA*>(p);
    }
};

template<>
struct shell_link_traits<wchar_t>
{
    typedef IShellLinkW interface_type;

    static IShellLinkW* create()
    {
        // CLSID_ShellLink
        const GUID clsid =
        {
            0x00021401, 0x0000, 0x0000,
            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
        };

        // IID_IShellLinkW
        const GUID iid =
        {
            0x000214F9, 0x0000, 0x0000,
            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
        };

        void* p;
        shell_check(
            ::CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, iid, &p)
        );

        return static_cast<IShellLinkW*>(p);
    }
};

template<class String>
class shell_link
{
public:
    typedef shell_link_traits<typename String::value_type> traits_type;
    typedef typename traits_type::interface_type interface_type;

    shell_link() : ptr_(traits_type::create())
    {
    }

    ~shell_link()
    {
        ptr_->Release();
    }

    void path(const String& ph)
    {
        shell_check(ptr_->SetPath(ph.c_str()));
    }

    void working_directory(const String& ph)
    {
        shell_check(ptr_->SetWorkingDirectory(ph.c_str()));
    }

    void save(const String& ph)
    {
        persist_file file(ptr_);
        file.save(ph.c_str());
    }

private:
    interface_type* ptr_;

    shell_link(const shell_link&);
    shell_link& operator=(const shell_link&);
};

template<class String>
inline error_code create_shell_link_template(
    const String& to_ph, const String& from_ph,
    const basic_shell_link_options<String>& options)
{
    try
    {
        shell_link<String> link;
        link.path(to_ph);
        if (!options.working_directory.empty())
            link.working_directory(options.working_directory);
        link.save(from_ph);
    }
    catch (const shell_error& e)
    {
        return make_error_code(e.result());
    }

    return error_code();
}

} } } // End namespaces detail, filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_WINDOWS_SHELL_LINK_HPP
