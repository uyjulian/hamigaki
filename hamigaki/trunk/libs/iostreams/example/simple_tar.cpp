//  simple_tar.cpp: a simple LZH compressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <boost/config.hpp>

#include <hamigaki/iostreams/device/tar_file.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/none.hpp>
#include <exception>
#include <iostream>

#include <sys/stat.h>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#elif defined(BOOST_HAS_UNISTD_H)
    #include <grp.h>
    #include <pwd.h>
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

        io_ex::tar_file_sink tar(argv[1]);

        for (int i = 2; i < argc; ++i)
        {
            io_ex::tar::header head;
            head.format = io_ex::tar::gnu;
            head.path = fs::path(argv[i], fs::native);
            if (fs::symbolic_link_exists(head.path))
            {
                head.type = io_ex::tar::type::symbolic_link;
                head.link_name = read_link(head.path);
            }
            else if (fs::is_directory(head.path))
                head.type = io_ex::tar::type::directory;
            else
                head.size = fs::file_size(head.path);
            head.modified_time =
                io_ex::tar::timestamp(fs::last_write_time(head.path));

#if defined(BOOST_WINDOWS)
            head.user_name = "root";
            head.group_name = "root";

            struct stat st;
            if (::stat(head.path.native_file_string().c_str(), &st) == 0)
            {
                head.mode = static_cast<
                    boost::uint16_t>(st.st_mode & io_ex::tar::mode::mask);
            }
#elif defined(BOOST_HAS_UNISTD_H)
            struct stat st;
            if (::lstat(head.path_string().c_str(), &st) == 0)
            {
                head.mode = static_cast<
                    boost::uint16_t>(st.st_mode & io_ex::tar::mode::mask);

                head.uid = st.st_uid;
                head.gid = st.st_gid;

                if (passwd* p = ::getpwuid(st.st_uid))
                    head.user_name = p->pw_name;

                if (group* p = ::getgrgid(st.st_gid))
                    head.group_name = p->gr_name;
            }
#endif

            tar.create_entry(head);

            if (head.is_regular())
            {
                io::copy(
                    io_ex::file_source(
                        head.path.native_file_string(),
                        std::ios_base::binary),
                    tar
                );
            }
        }
        tar.write_end_mark();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
