//  simple_unlha.cpp: a simple LZH decompressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.


// Security warning:
// This program never check the validity of paths in the archive.
// See http://www.forest.impress.co.jp/article/2004/07/30/arcsecurity.html .
// (The above link is Japanese site)


// Note:
// SjLj exception handling is too slow.
// Also boost::iostreams::file_source never throw any exceptions
// even if some error are detected.
// So we use boost::iostreams::file_source instead of Hamigaki's one.
#if defined(__GNUC__) && defined(__USING_SJLJ_EXCEPTIONS__)
    #define USE_BOOST_IOSTREAMS_FILE
#endif

#include <hamigaki/iostreams/device/lzh_file.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

#if defined(USE_BOOST_IOSTREAMS_FILE)
    #include <boost/iostreams/device/file.hpp>
    #define IOEX io
#else
    #include <hamigaki/iostreams/device/file.hpp>
    #define IOEX io_ex
#endif

namespace io_ex = hamigaki::iostreams;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
            return 1;

        std::setlocale(LC_ALL, "");
        fs::path::default_name_check(fs::no_check);

        typedef io_ex::basic_lzh_file_source<IOEX::file_source> lzh_type;
        lzh_type lzh(IOEX::file_source(argv[1], std::ios_base::binary));

        do
        {
            const io_ex::lha::basic_header& head = lzh.header();
            if (head.attributes & io_ex::lha::attributes::directory)
                fs::create_directories(head.path);
            else
            {
                fs::create_directories(head.path.branch_path());

                io::copy(
                    lzh,
                    IOEX::file_sink(
                        head.path.native_file_string(), std::ios_base::binary),
                    1024*8
                );
            }

            fs::last_write_time(head.path, head.update_time);

        } while (lzh.next_entry());

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
