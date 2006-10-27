//  simple_unzip.cpp: a simple zip decompressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/device/zip_file.hpp>
#include <exception>
#include <iostream>

namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
            return 1;

        io_ex::zip_file_source zip(argv[1]);

        while (zip.next_entry())
        {
            const io_ex::zip::header& head = zip.header();

            std::cout << head.path.string() << '\n';
            if (head.is_symbolic_link())
                std::cout << "-> " << head.link_path.string() << '\n';
            else if (!head.is_directory())
            {
                char buf[256];
                std::streamsize n;
                while (n = zip.read(buf, sizeof(buf)), n >= 0)
                {
                    if (n)
                        std::cout.write(buf, n);
                }
                std::cout << '\n';
            }

            std::cout << "--------------------------------" << std::endl;
        }

        std::cout.flush();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
