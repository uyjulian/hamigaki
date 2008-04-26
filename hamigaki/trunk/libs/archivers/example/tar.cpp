// tar.cpp: a simple tar archiver

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <boost/config.hpp>

#include <hamigaki/archivers/tar_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <hamigaki/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

#if defined(BOOST_WINDOWS)
#elif defined(BOOST_HAS_UNISTD_H)
    #include <grp.h>
    #include <pwd.h>
    #include <unistd.h>
#endif

namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;
namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
        {
            std::cerr << "Usage: tar (archive) (filename) ..." << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        // file_descriptor_sink supports 64bit offset
        ar::basic_tar_file_sink<io_ex::file_descriptor_sink>
            tar((io_ex::file_descriptor_sink(
                std::string(argv[1]), BOOST_IOS::binary)));

        for (int i = 2; i < argc; ++i)
        {
            ar::tar::header head;
            head.path = fs::path(argv[i]);

            const fs_ex::file_status& s = fs_ex::symlink_status(head.path);

            if (is_symlink(s))
            {
                head.type_flag = ar::tar::type_flag::symlink;
                head.link_path = fs_ex::symlink_target(head.path);
            }
            else if (is_directory(s))
                head.type_flag = ar::tar::type_flag::directory;
            else
                head.file_size = s.file_size();

            head.modified_time = s.last_write_time();
            head.access_time = s.last_access_time();

            if (s.has_last_change_time())
                head.change_time = s.last_change_time();

            if (s.has_permissions())
                head.permissions = s.permissions();

            if (s.has_uid() && s.has_gid())
            {
                head.uid = s.uid();
                head.gid = s.gid();
            }

#if defined(BOOST_WINDOWS)
            head.user_name = "root";
            head.group_name = "root";
#elif defined(BOOST_HAS_UNISTD_H)
            if (s.has_uid())
            {
                if (passwd* p = ::getpwuid(s.uid()))
                    head.user_name = p->pw_name;
            }
            if (s.has_gid())
            {
                if (group* p = ::getgrgid(s.gid()))
                    head.group_name = p->gr_name;
            }
#endif

            tar.create_entry(head);

            if (!fs::is_directory(head.path))
            {
                io::copy(
                    io_ex::file_descriptor_source(
                        head.path.file_string(),
                        std::ios_base::binary),
                    tar
                );
            }
        }
        tar.close_archive();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
