// delete_zip.cpp: delete the specified entry from the ZIP file

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <hamigaki/archivers/raw_zip_file.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

namespace ar = hamigaki::archivers;
namespace fs = boost::filesystem;
namespace io = boost::iostreams;

bool is_parent_of(const fs::path& parent, const fs::path& child)
{
    typedef fs::path::iterator iter_type;

    iter_type p_beg = parent.begin();
    iter_type p_end = parent.end();

    iter_type c_beg = child.begin();
    iter_type c_end = child.end();

    while ((p_beg != p_end) && (c_beg != c_end))
    {
        if (*p_beg != *c_beg)
            return false;

        ++p_beg;
        ++c_beg;
    }

    return (c_beg != c_end);
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: delete_zip (ZIP file) (filename)" << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        fs::path zip_name(argv[1], fs::native);
        const fs::path& bak_name = change_extension(zip_name, ".bak");
        rename(zip_name, bak_name);

        fs::path del_name(argv[2], fs::native);

        {
            ar::raw_zip_file_source src(bak_name.file_string());
            ar::raw_zip_file_sink sink(zip_name.file_string());

            while (src.next_entry())
            {
                const ar::zip::header& head = src.header();

                if ((head.path == del_name) || is_parent_of(del_name,head.path))
                    continue;

                sink.create_entry(head);

                if (head.compressed_size != 0)
                    io::copy(src, sink);
            }
            sink.close_archive();
        }

        remove(bak_name);
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
