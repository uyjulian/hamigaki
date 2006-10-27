//  simple_zip.cpp: a simple ZIP compressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <boost/config.hpp>

#include <hamigaki/iostreams/device/zip_file.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/none.hpp>
#include <exception>
#include <iostream>

#include <sys/stat.h>

#if defined(BOOST_HAS_UNISTD_H)
    #include <unistd.h>
#endif

namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

#if defined(BOOST_HAS_UNISTD_H)
fs::path read_link(const fs::path& ph)
{
    const std::string& s = ph.native_file_string();

    struct stat st;
    if (::lstat(s.c_str(), &st) != 0)
        throw std::runtime_error("lstat error");

    boost::scoped_array<char> buf(new char[st.st_size+1]);
    ::ssize_t amt = ::readlink(s.c_str(), &buf[0], st.st_size);
    if (amt != static_cast< ::ssize_t>(st.st_size))
        throw std::runtime_error("bad symbolic link");

    buf[st.st_size] = '\0';
    return fs::path(&buf[0], fs::native);
}
#else
fs::path read_link(const fs::path& ph)
{
    return fs::path();
}
#endif

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
            return 1;

        io_ex::zip_file_sink zip(argv[1]);

        for (int i = 2; i < argc; ++i)
        {
            io_ex::zip::header head;
            head.method = 8;
            head.path = fs::path(argv[i], fs::native);
            if (fs::symbolic_link_exists(head.path))
                head.link_path = read_link(head.path);
            else if (fs::is_directory(head.path))
                head.attributes = io_ex::msdos_attributes::directory;
            else
                head.file_size = fs::file_size(head.path);
            head.update_time = fs::last_write_time(head.path);

            struct stat st;
#if defined(BOOST_HAS_UNISTD_H)
            if (::lstat(head.path.native_file_string().c_str(), &st) == 0)
#else
            if (::stat(head.path.native_file_string().c_str(), &st) == 0)
#endif
            {
                head.permission = static_cast<boost::uint16_t>(st.st_mode);
                head.modified_time = st.st_mtime;
                head.access_time = st.st_atime;
#if defined(BOOST_WINDOWS)
                head.creation_time = st.st_ctime;
#endif
            }

            zip.create_entry(head);

            if (!head.is_symbolic_link() && !head.is_directory())
            {
                try
                {
                    io::copy(
                        io_ex::file_source(
                            head.path.native_file_string(),
                            std::ios_base::binary),
                        zip
                    );
                }
                catch (const io_ex::give_up_compression&)
                {
                    zip.rewind_entry();
                    io::copy(
                        io_ex::file_source(
                            head.path.native_file_string(),
                            std::ios_base::binary),
                        zip
                    );
                }
            }
        }
        zip.write_end_mark();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
