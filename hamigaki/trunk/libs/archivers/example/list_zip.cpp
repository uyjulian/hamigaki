// list_zip.cpp: show the file list in the ZIP file

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/zip_file.hpp>
#include <hamigaki/iostreams/device/file_descriptor.hpp>
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
            std::cerr << "Usage: unzip (archive)" << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        // file_descriptor_source supports 64bit offset
        ar::basic_zip_file_source<io_ex::file_descriptor_source>
            zip((io_ex::file_descriptor_source(
                std::string(argv[1]), BOOST_IOS::in|BOOST_IOS::binary)));

        while (zip.next_entry())
        {
            const ar::zip::header& head = zip.header();
            std::cout << head.path.string() << '\n';
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
