//  simple_zip.cpp: a simple ZIP compressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#include <boost/config.hpp>

#include <hamigaki/iostreams/device/zip_file.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/none.hpp>
#include <exception>
#include <iostream>

#include <sys/stat.h>

namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
            return 1;

        io_ex::zip_file_sink zip(argv[1]);

        for (int i = 2; i < argc; ++i)
        {
            io_ex::zip::header head;
            head.method = 8;
//head.method = 0;
            head.path = fs::path(argv[i], fs::native);
            if (fs::is_directory(head.path))
                head.attributes = io_ex::msdos_attributes::directory;
            head.update_time = fs::last_write_time(head.path);

            struct stat st;
            if (::stat(head.path.native_file_string().c_str(), &st) == 0)
                head.permission = static_cast<boost::uint16_t>(st.st_mode);

            zip.create_entry(head);

            if (!head.is_directory())
            {
                io::copy(
                    io_ex::file_source(
                        head.path.native_file_string(),
                        std::ios_base::binary),
                    zip
                );
            }
        }
        zip.write_end_mark();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
