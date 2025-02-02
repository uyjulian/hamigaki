// unzip.cpp: a simple ZIP decompressing program

// Copyright Takeshi Mouri 2006-2018.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.


// Security warning:
// This program never check the validity of paths in the archive.
// See http://www.forest.impress.co.jp/article/2004/07/30/arcsecurity.html .
// (The above link is Japanese site)


#include <hamigaki/archivers/zip_file.hpp>
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

template<class Path>
bool is_valid_path(const Path& ph)
{
#if !defined(HAMIGAKI_ALLOW_DIRECTORY_TRAVERSAL)
    if (ph.has_root_name() || ph.has_root_directory())
        return false;
    for (typename Path::iterator it = ph.begin(); it != ph.end(); ++it)
    {
        if (*it == "..")
            return false;
    }
#endif
    return true;
}

inline fs_ex::timestamp make_timestamp(std::time_t t)
{
    return fs_ex::timestamp::from_time_t(t);
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: unzip (archive)" << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        ar::zip_file_source zip(argv[1]);

        while (zip.next_entry())
        {
            const ar::zip::header& head = zip.header();

            std::cout << head.path.string() << '\n';
            if (!is_valid_path(head.path))
            {
                std::cerr << "Warning: invalid path" << '\n';
                continue;
            }

            if (!head.link_path.empty())
            {
                if (::has_parent_path(head.path))
                    fs::create_directories(head.path.branch_path());

                fs_ex::create_symlink(head.link_path, head.path);
            }
            else if (head.is_directory())
                fs::create_directories(head.path);
            else
            {
                if (::has_parent_path(head.path))
                    fs::create_directories(head.path.branch_path());

                io::copy(
                    zip,
                    io_ex::file_sink(
                        head.path.file_string(), std::ios_base::binary)
                );
            }

            // Note:
            // The POSIX chown() clears S_ISUID/S_ISGID bits.
            // So, we must call change_symlink_owner()
            // before calling change_permissions().
            fs_ex::error_code ec;
            boost::optional<boost::intmax_t> uid;
            if (head.uid)
                uid = head.uid.get();
            boost::optional<boost::intmax_t> gid;
            if (head.gid)
                gid = head.gid.get();
            fs_ex::change_symlink_owner(head.path, uid, gid, ec);

            fs_ex::change_attributes(head.path, head.attributes, ec);
            fs_ex::change_permissions(head.path, head.permissions, ec);

            if (head.modified_time)
            {
                fs_ex::last_write_time(
                    head.path, make_timestamp(*head.modified_time));
            }
            if (head.access_time)
            {
                fs_ex::last_access_time(
                    head.path, make_timestamp(*head.access_time));
            }
            if (head.creation_time)
            {
                fs_ex::creation_time(
                    head.path, make_timestamp(*head.creation_time));
            }
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
