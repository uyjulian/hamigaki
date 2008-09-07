// uniso.cpp: a simple ISO image extractor program

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.


// Security warning:
// This program never check the validity of paths in the archive.
// See http://www.forest.impress.co.jp/article/2004/07/30/arcsecurity.html .
// (The above link is Japanese site)


#include <hamigaki/archivers/iso_file.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/version.hpp>
#include <clocale>
#include <exception>
#include <functional>
#include <iostream>

namespace ar = hamigaki::archivers;
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
            std::cerr << "Usage: uniso (archive)" << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        ar::iso_file_source iso(argv[1]);

        typedef std::vector<ar::iso::volume_desc> descs_type;
        const descs_type& descs = iso.volume_descs();

        descs_type::const_iterator rr = std::find_if(
            descs.begin(), descs.end(),
            std::mem_fun_ref(&ar::iso::volume_desc::is_rock_ridge)
        );

        descs_type::const_iterator jol = std::find_if(
            descs.begin(), descs.end(),
            std::mem_fun_ref(&ar::iso::volume_desc::is_joliet)
        );

        if (rr != descs.end())
            iso.select_volume_desc(rr - descs.begin());
        else if (jol != descs.end())
            iso.select_volume_desc(jol - descs.begin());
        else
            iso.select_volume_desc(0);

        while (iso.next_entry())
        {
            const ar::iso::header& head = iso.header();

            std::cout << head.path.string() << '\n';

            if (head.is_symlink())
                std::cout << "-> " << head.link_path.string() << '\n';
            else if (head.is_directory())
                fs::create_directories(head.path);
            else if (head.is_regular())
            {
                if (::has_parent_path(head.path))
                    fs::create_directories(head.path.branch_path());

                io::copy(
                    iso,
                    io_ex::file_sink(
                        head.path.file_string(), std::ios_base::binary)
                );
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
