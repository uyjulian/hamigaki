// iso.cpp: a simple ISO image archiver program

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

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

        ar::iso_file_sink iso(argv[1]);

        {
            ar::iso::volume_desc desc;
            desc.rrip = ar::iso::rrip_1991a;
            iso.add_volume_desc(desc);
        }
        {
            ar::iso::volume_desc jol_desc;
            jol_desc.set_joliet();
            iso.add_volume_desc(jol_desc);
        }

        for (int i = 2; i < argc; ++i)
        {
            ar::iso::header head;
            head.path = fs::path(argv[i]);

            const fs_ex::file_status& s = fs_ex::status(head.path);

            head.type(s.type());
            if (is_regular(s))
                head.file_size = s.file_size();

            ar::iso::posix::file_attributes attr;
            if (s.has_permissions())
                attr.permissions = s.permissions();
            else if (is_directory(s))
                attr.permissions = fs_ex::file_permissions::directory | 0755u;
            else
                attr.permissions = fs_ex::file_permissions::regular | 0644u;
            attr.links = 1u;
            if (s.has_uid())
                attr.uid = s.uid();
            if (s.has_gid())
                attr.gid = s.gid();
            attr.serial_no = static_cast<unsigned>(i);
            head.attributes = attr;

            typedef ar::iso::date_time time_time;
            head.last_write_time =
                time_time::from_timestamp(s.last_write_time());
            head.last_access_time =
                time_time::from_timestamp(s.last_access_time());
            if (s.has_last_change_time())
            {
                head.last_change_time =
                    time_time::from_timestamp(s.last_change_time());
            }

            iso.create_entry(head);

            if (is_regular(s))
            {
                io::copy(
                    io_ex::file_source(
                        head.path.file_string(),
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
