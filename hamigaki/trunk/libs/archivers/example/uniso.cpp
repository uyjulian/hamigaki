//  uniso.cpp: a simple ISO image extractor program

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.


// Security warning:
// This program never check the validity of paths in the archive.
// See http://www.forest.impress.co.jp/article/2004/07/30/arcsecurity.html .
// (The above link is Japanese site)


#include <hamigaki/archivers/iso_image_file.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

namespace ar = hamigaki::archivers;
namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

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
        fs::path::default_name_check(fs::no_check);

        ar::iso_image_file_source iso(argv[1]);

        while (iso.next_entry())
        {
            const ar::iso9660::header& head = iso.header();

            std::cout << head.path.string() << '\n';

            if (head.is_directory())
                fs::create_directories(head.path);
            else
            {
                if (head.path.has_branch_path())
                    fs::create_directories(head.path.branch_path());

                io::copy(
                    iso,
                    io_ex::file_sink(
                        head.path.native_file_string(), std::ios_base::binary)
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
