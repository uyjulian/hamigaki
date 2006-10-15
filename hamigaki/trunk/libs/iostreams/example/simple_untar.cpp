//  simple_untar.cpp: a simple tar decompressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <hamigaki/iostreams/device/tar_file.hpp>
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

        fs::path::default_name_check(fs::no_check);

        io_ex::tar_file_source tar(argv[1]);

        do
        {
            const io_ex::tar::header& head = tar.header();

            std::cout << head.path.string() << '\n';

            if (head.type == io_ex::tar::type::link)
                std::cout << "is a link to " << head.link_name.string() << '\n';
            else if (head.type == io_ex::tar::type::symbolic_link)
            {
                std::cout
                    << "is a symbolic-link to "
                    << head.link_name.string() << '\n';
            }
            else if (head.type == io_ex::tar::type::directory)
            {
                std::cout << "is a directroy\n";
            }
            else if (head.is_device())
            {
                if (head.type == io_ex::tar::type::char_device)
                    std::cout << "is a character device\n";
                else
                    std::cout << "is a block device\n";

                std::cout << "major=" << head.dev_major << '\n';
                std::cout << "minor=" << head.dev_minor << '\n';
            }
            else if (head.is_regular())
            {
                char buf[256];
                std::streamsize n;
                while (n = tar.read(buf, sizeof(buf)), n >= 0)
                {
                    if (n)
                        std::cout.write(buf, n);
                }
                std::cout << '\n';
            }
            std::cout << "--------------------------------" << std::endl;

        } while (tar.next_entry());

        std::cout.flush();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
