// make_tbz.cpp: tar.bz2 archive writer for Hamigaki release tool

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/tools/release/ for library home page.

#include "make_tbz.hpp"
#include "file_types.hpp"
#include <hamigaki/archivers/tbz2_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/copy.hpp>
#include <ostream>

namespace fs = boost::filesystem;
namespace io = boost::iostreams;
namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;

void make_tbz2_archive(std::ostream& logs, const std::string& ver)
{
    ar::tbz2_file_sink tbz2("hamigaki_" + ver + ".tar.bz2");

    fs::recursive_directory_iterator beg("hamigaki_" + ver);
    fs::recursive_directory_iterator end;

    for (; beg != end; ++beg)
    {
        const fs::path& ph = beg->path();
        const fs_ex::file_status s = fs_ex::status(ph);

        logs << ph << std::endl;

        ar::tar::header head;
        head.path = ph;

        if (s.has_permissions())
            head.permissions = s.permissions();
        else if (fs_ex::is_directory(s))
        {
            head.permissions = 0755;
            head.type_flag = ar::tar::type_flag::directory;
        }

        if (fs_ex::is_regular(s))
        {
            head.file_size = s.file_size();

            if (is_executable(fs::extension(ph)))
                head.permissions |= 0111;
        }

        head.modified_time = s.last_write_time();
        head.access_time = s.last_access_time();

        if (s.has_last_change_time())
            head.change_time = s.last_change_time();

        tbz2.create_entry(head);

        if (fs_ex::is_regular(s))
        {
            fs::ifstream is(ph, std::ios_base::binary);
            io::copy(is, tbz2);
        }
        else
            tbz2.close();
    }
    tbz2.close_archive();
}
