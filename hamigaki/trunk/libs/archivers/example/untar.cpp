// untar.cpp: a simple ZIP decompressing program

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.


// Security warning:
// This program never check the validity of paths in the archive.
// See http://www.forest.impress.co.jp/article/2004/07/30/arcsecurity.html .
// (The above link is Japanese site)


#include <hamigaki/archivers/tar_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;
namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

template<class Path>
inline bool has_parent_path(const Path& ph)
{
#if BOOST_VERSION < 103600
    return ph.has_branch_path();
#else
    return ph.has_parent_path();
#endif
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: untar (archive)" << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        ar::tar_file_source tar(argv[1]);

        while (tar.next_entry())
        {
            const ar::tar::header& head = tar.header();

            std::cout << head.path.string() << '\n';

            if (head.type_flag == ar::tar::type_flag::link)
            {
                if (::has_parent_path(head.path))
                    fs::create_directories(head.path.branch_path());

                fs_ex::create_hard_link(head.link_path, head.path);
            }
            else if (head.type_flag == ar::tar::type_flag::symlink)
            {
                if (::has_parent_path(head.path))
                    fs::create_directories(head.path.branch_path());

                fs_ex::create_symlink(head.link_path, head.path);
            }
            else if (head.type_flag == ar::tar::type_flag::directory)
                fs::create_directories(head.path);
            else if (head.type_flag == ar::tar::type_flag::char_device)
                std::cerr << "Warning: skip character device\n";
            else if (head.type_flag == ar::tar::type_flag::block_device)
                std::cerr << "Warning: skip block device\n";
            else if (head.type_flag == ar::tar::type_flag::fifo)
                std::cerr << "Warning: skip FIFO file\n";
            // Note: All unknown types are treated as a regular file.
            else
            {
                if (::has_parent_path(head.path))
                    fs::create_directories(head.path.branch_path());

                io::copy(
                    tar,
                    io_ex::file_sink(
                        head.path.file_string(), std::ios_base::binary)
                );
            }

            // Note:
            // The POSIX chown() clears S_ISUID/S_ISGID bits.
            // So, we must call change_symlink_owner()
            // before calling change_permissions().
            fs_ex::error_code ec;
            fs_ex::change_symlink_owner(head.path, head.uid, head.gid, ec);

            fs_ex::change_permissions(head.path, head.permissions, ec);

            if (head.modified_time)
                fs_ex::last_write_time(head.path, *head.modified_time);
            if (head.access_time)
                fs_ex::last_access_time(head.path, *head.access_time);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
