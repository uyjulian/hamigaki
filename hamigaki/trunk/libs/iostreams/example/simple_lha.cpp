//  simple_lha.cpp: a simple LZH compressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.


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
inline boost::uint64_t to_uint64(const ::FILETIME& ft)
{
    return
        (static_cast<boost::uint64_t>(ft.dwHighDateTime) << 32) |
        static_cast<boost::uint64_t>(ft.dwLowDateTime);
}

void get_timestamp_impl(
    ::HANDLE handle, io_ex::lha::windows_timestamp& ts)
{
    ::FILETIME creation_time, last_write_time, last_access_time;
    ::GetFileTime(handle, &creation_time,
        &last_access_time, &last_write_time);

    ts.creation_time = to_uint64(creation_time);
    ts.last_write_time = to_uint64(last_write_time);
    ts.last_access_time = to_uint64(last_access_time);
}

io_ex::lha::windows_timestamp get_file_timestamp(const fs::path& ph)
{
    ::HANDLE handle = ::CreateFileA(
        ph.native_file_string().c_str(), GENERIC_READ, 0, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (handle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("CreateFile error");
    io_ex::lha::windows_timestamp ts;
    get_timestamp_impl(handle, ts);
    ::CloseHandle(handle);
    return ts;
}

boost::optional<io_ex::lha::windows_timestamp>
get_directory_timestamp(const fs::path& ph)
{
    ::HANDLE handle = ::CreateFileA(
        ph.native_file_string().c_str(), GENERIC_READ, 0, 0,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
    // Win9X does not support FILE_FLAG_BACKUP_SEMANTICS flag
    if (handle == INVALID_HANDLE_VALUE)
        return boost::none;
    io_ex::lha::windows_timestamp ts;
    get_timestamp_impl(handle, ts);
    ::CloseHandle(handle);
    return ts;
}
#endif

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
            return 1;

        std::setlocale(LC_ALL, "");
        fs::path::default_name_check(fs::no_check);

        typedef io_ex::basic_lzh_file_sink<IOEX::file_sink> lzh_type;
        lzh_type lzh(IOEX::file_sink(argv[1], std::ios_base::binary));

        for (int i = 2; i < argc; ++i)
        {
            io_ex::lha::header head;
            head.path = fs::path(argv[i], fs::native);
            if (fs::is_directory(head.path))
                head.attributes = io_ex::lha::attributes::directory;
            else
                head.file_size = fs::file_size(head.path);
            head.update_time = fs::last_write_time(head.path);

#if defined(BOOST_WINDOWS)
            head.attributes = ::GetFileAttributes(head.path_string().c_str());
            head.code_page = ::GetACP();
            if (fs::is_directory(head.path))
                head.timestamp = get_directory_timestamp(head.path);
            else
                head.timestamp = get_file_timestamp(head.path);
#elif defined(BOOST_HAS_UNISTD_H)
            struct stat st;
            if (::stat(head.path_string().c_str(), &st) == 0)
                head.permission = st.st_mode;
#endif

            lzh.create_entry(head);

            if (!fs::is_directory(head.path))
            {
                try
                {
                    io::copy(
                        IOEX::file_source(
                            head.path.native_file_string(),
                            std::ios_base::binary),
                        lzh
                    );
                }
                catch (const io_ex::lha::give_up_compression&)
                {
                    lzh.rewind_entry();

                    io::copy(
                        IOEX::file_source(
                            head.path.native_file_string(),
                            std::ios_base::binary),
                        lzh
                    );
                }
            }
        }
        lzh.write_end_mark();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
