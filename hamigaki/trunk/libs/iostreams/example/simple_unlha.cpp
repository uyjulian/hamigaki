//  simple_unlha.cpp: a simple LZH decompressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.


// Security warning:
// This program never check the validity of paths in the archive.
// See http://www.forest.impress.co.jp/article/2004/07/30/arcsecurity.html .
// (The above link is Japanese site)


// Note:
// SjLj exception handling is too slow.
// Also boost::iostreams::file_source never throw any exceptions
// even if some error are detected.
// So we use boost::iostreams::file_source instead of Hamigaki's one.
#if defined(__GNUC__) && defined(__USING_SJLJ_EXCEPTIONS__)
    #define USE_BOOST_IOSTREAMS_FILE
#endif

#include <boost/config.hpp>

#include <hamigaki/iostreams/device/lzh_file.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

#if defined(USE_BOOST_IOSTREAMS_FILE)
    #include <boost/iostreams/device/file.hpp>
    #define IOEX io
#else
    #include <hamigaki/iostreams/device/file.hpp>
    #define IOEX io_ex
#endif

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#elif defined(BOOST_HAS_UNISTD_H)
    #include <sys/stat.h>
#endif

namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

#if defined(BOOST_WINDOWS)
::FILETIME make_file_time(boost::uint64_t n)
{
    ::FILETIME ft;
    ft.dwLowDateTime = static_cast<boost::uint32_t>(n);
    ft.dwHighDateTime = static_cast<boost::uint32_t>(n >> 32);
    return ft;
}

void set_timestamp_impl(
    ::HANDLE handle, const io_ex::lha::windows_timestamp& ts)
{
    ::FILETIME creation_time = make_file_time(ts.creation_time);
    ::FILETIME last_write_time = make_file_time(ts.last_write_time);
    ::FILETIME last_access_time = make_file_time(ts.last_access_time);

    ::SetFileTime(handle, &creation_time,
        &last_access_time, &last_write_time);
}

void set_file_timestamp(
    const fs::path& ph, const io_ex::lha::windows_timestamp& ts)
{
    ::HANDLE handle = ::CreateFileA(
        ph.native_file_string().c_str(), GENERIC_WRITE, 0, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("CreateFile error");
    set_timestamp_impl(handle, ts);
    ::CloseHandle(handle);
}

void set_directory_timestamp(
    const fs::path& ph, const io_ex::lha::windows_timestamp& ts)
{
    ::HANDLE handle = ::CreateFileA(
        ph.native_directory_string().c_str(), GENERIC_WRITE, 0, 0,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
    // Win9X does not support FILE_FLAG_BACKUP_SEMANTICS flag
    if (handle == INVALID_HANDLE_VALUE)
        return;
    set_timestamp_impl(handle, ts);
    ::CloseHandle(handle);
}
#endif

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
            return 1;

        std::setlocale(LC_ALL, "");
        fs::path::default_name_check(fs::no_check);

        typedef io_ex::basic_lzh_file_source<IOEX::file_source> lzh_type;
        lzh_type lzh(IOEX::file_source(argv[1], std::ios_base::binary));

        while (lzh.next_entry())
        {
            const io_ex::lha::header& head = lzh.header();
            if (head.is_directory())
                fs::create_directories(head.path);
            else
            {
                fs::create_directories(head.path.branch_path());

                io::copy(
                    lzh,
                    IOEX::file_sink(
                        head.path.native_file_string(), std::ios_base::binary),
                    1024*8
                );
            }

#if defined(BOOST_WINDOWS)
            {
                ::DWORD attr = head.attributes & io_ex::lha::attributes::mask;
                ::SetFileAttributes(head.path_string().c_str(), attr);
            }
#elif defined(BOOST_HAS_UNISTD_H)
            if (head.permission)
                ::chmod(head.path_string().c_str(), *head.permission);
            if (head.owner)
            {
                ::chown(
                    head.path_string().c_str(),
                    head.owner->uid, head.owner->gid);
            }
#endif

#if defined(BOOST_WINDOWS)
            if (head.timestamp)
            {
                if (head.is_directory())
                    set_directory_timestamp(head.path, *head.timestamp);
                else
                    set_file_timestamp(head.path, *head.timestamp);
            }
            else
#endif
            if (!head.is_directory())
                fs::last_write_time(head.path, head.update_time);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
