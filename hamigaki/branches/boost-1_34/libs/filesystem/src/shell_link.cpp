//  shell_link.cpp: the shell link operations

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#define HAMIGAKI_FILESYSTEM_SOURCE
#define BOOST_ALL_NO_LIB
#define NOMINMAX

#if !defined(_WIN32_WINNT)
    #define _WIN32_WINNT 0x0500
#endif

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)

#include <hamigaki/filesystem/operations.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

#include <windows.h>
#include <objbase.h>
#include <objidl.h>
#include <shlobj.h>

namespace hamigaki { namespace filesystem {

namespace
{

class shell_error : public std::exception
{
public:
    shell_error(::HRESULT res) : res_(res)
    {
    }

    ~shell_error() throw() // virtual
    {
    }

    ::HRESULT result() const
    {
        return res_;
    }

private:
    ::HRESULT res_;
};

inline void shell_check(::HRESULT res)
{
    if (FAILED(res))
        throw shell_error(res);
}

// TODO: copy from symlink.cpp
inline std::wstring to_wide(const char* s, int n)
{
    int size = ::MultiByteToWideChar(CP_ACP, 0, s, n, 0, 0);
    if (size == 0)
        return std::wstring();

    std::vector<wchar_t> buf(size);
    size = ::MultiByteToWideChar(CP_ACP, 0, s, n, &buf[0], size);
    if (size == 0)
        return std::wstring();
    buf.resize(size);

    return std::wstring(&buf[0], size);
}

inline std::wstring to_wide(const std::string& s)
{
    return to_wide(s.c_str(), s.size());
}

class persist_file : boost::noncopyable
{
public:
    explicit persist_file(::IUnknown* p)
    {
        // IID_IPersistFile
        const ::GUID iid =
        {
            0x0000010B, 0x0000, 0x0000,
            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
        };

        void* tmp;
        shell_check(p->QueryInterface(iid, &tmp));
        ptr_ = static_cast< ::IPersistFile*>(tmp);
    }

    ~persist_file()
    {
        ptr_->Release();
    }

    void save(const boost::filesystem::path& ph)
    {
        ptr_->Save(to_wide(ph.native_file_string()).c_str(), TRUE);
    }

private:
    ::IPersistFile* ptr_;
};

class shell_link : boost::noncopyable
{
public:
    shell_link()
    {
        // CLSID_ShellLink
        const ::GUID clsid =
        {
            0x00021401, 0x0000, 0x0000,
            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
        };

        // IID_IShellLinkA
        const ::GUID iid =
        {
            0x000214EE, 0x0000, 0x0000,
            { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 }
        };

        void* p;
        shell_check(
            ::CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, iid, &p) );

        ptr_ = static_cast< ::IShellLinkA*>(p);
    }

    ~shell_link()
    {
        ptr_->Release();
    }

    void path(const boost::filesystem::path& ph)
    {
        shell_check(ptr_->SetPath(ph.native_file_string().c_str()));
    }

    void working_directory(const boost::filesystem::path& ph)
    {
        shell_check(
            ptr_->SetWorkingDirectory(ph.native_directory_string().c_str()) );
    }

    void save(const boost::filesystem::path& ph)
    {
        persist_file file(ptr_);
        file.save(ph);
    }

private:
    ::IShellLinkA* ptr_;
};

} // namespace

HAMIGAKI_FILESYSTEM_DECL
int create_shell_link(
    const boost::filesystem::path& old_fp,
    const boost::filesystem::path& new_fp, int& ec)
{
    ec = 0;

    try
    {
        boost::filesystem::path ph = system_complete(old_fp);

        shell_link link;
        link.path(ph);
        link.working_directory(ph.branch_path());
        link.save(new_fp);
    }
    catch (const shell_error& e)
    {
        ec = e.result();
    }

    return ec;
}

} } // End namespaces filesystem, hamigaki.

#endif // defined(BOOST_WINDOWS)
