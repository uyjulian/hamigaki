// make_zip.cpp: ZIP archive writer for Hamigaki release tool

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/tools/release/ for library home page.

#include "make_zip.hpp"
#include "file_types.hpp"
#include <hamigaki/archivers/zip_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/copy.hpp>
#include <ostream>

namespace fs = boost::filesystem;
namespace io = boost::iostreams;
namespace ar = hamigaki::archivers;
namespace fs_ex = hamigaki::filesystem;

void make_zip_archive(std::ostream& logs, const std::string& ver)
{
    ar::zip_file_sink zip("hamigaki_" + ver + ".zip");

    fs::recursive_directory_iterator beg("hamigaki_" + ver);
    fs::recursive_directory_iterator end;

    for (; beg != end; ++beg)
    {
        const fs::path& ph = beg->path();
        const fs_ex::file_status s = fs_ex::status(ph);

        logs << ph << std::endl;

        ar::zip::header head;
        head.path = ph;

        if (is_directory(s))
            head.attributes = ar::msdos::attributes::directory;
        else
            head.file_size = s.file_size();
        head.update_time = s.last_write_time().to_time_t();

        if (s.has_attributes())
            head.attributes = s.attributes();

        if (s.has_creation_time())
        {
            head.modified_time = s.last_write_time().to_time_t();
            head.access_time = s.last_access_time().to_time_t();
            head.creation_time = s.creation_time().to_time_t();
        }

        if (s.has_permissions())
            head.permissions = s.permissions();

        if (fs_ex::is_regular(s) && is_executable(fs::extension(ph)))
            head.permissions |= 0111;

        zip.create_entry(head);

        if (is_regular(s))
        {
            try
            {
                fs::ifstream is(ph, std::ios_base::binary);
                io::copy(is, zip);
            }
            catch (const ar::give_up_compression&)
            {
                zip.rewind_entry();

                fs::ifstream is(ph, std::ios_base::binary);
                io::copy(is, zip);
            }
        }
        else
            zip.close();
    }
    zip.close_archive();
}
