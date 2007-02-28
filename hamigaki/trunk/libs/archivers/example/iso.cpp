//  iso.cpp: a simple ISO image archiver program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/iso_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

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
            std::cerr << "Usage: iso (archive) (filename) ..." << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");
        fs::path::default_name_check(fs::no_check);

        ar::iso_file_sink iso(argv[1]);

        for (int i = 2; i < argc; ++i)
        {
            ar::iso::header head;
            head.path = fs::path(argv[i], fs::native);

            const fs_ex::file_status& s = fs_ex::status(head.path);

            head.type(s.type());
            if (is_regular(s))
                head.file_size = s.file_size();

            iso.create_entry(head);

            if (is_regular(s))
            {
                io::copy(
                    io_ex::file_source(
                        head.path.native_file_string(),
                        std::ios_base::binary),
                    iso
                );
            }
        }
        iso.close_archive();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
