// untar.cpp: a simple tar extracting program (Unicode version)

// Copyright Takeshi Mouri 2008-2018.
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
#include <boost/filesystem/fstream.hpp>
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

template<class Path>
bool is_valid_path(const Path& ph)
{
#if !defined(HAMIGAKI_ALLOW_DIRECTORY_TRAVERSAL)
    if (ph.has_root_name() || ph.has_root_directory())
        return false;
    for (typename Path::iterator it = ph.begin(); it != ph.end(); ++it)
    {
        if (*it == L"..")
            return false;
    }
#endif
    return true;
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

        ar::wtar_file_source tar(argv[1]);

        while (tar.next_entry())
        {
            const ar::tar::wheader& head = tar.header();

            std::wcout << head.path << std::endl;
            if (!is_valid_path(head.path))
            {
                std::cerr << "Warning: invalid path" << '\n';
                continue;
            }

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

                fs::ofstream os(head.path, std::ios_base::binary);
                io::copy(tar, os);
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
