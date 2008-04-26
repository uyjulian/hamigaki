// zip.cpp: a simple ZIP compressing program

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <boost/config.hpp>

#include <hamigaki/archivers/zip_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <hamigaki/iostreams/device/file_descriptor.hpp>
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
            std::cerr << "Usage: zip (archive) (filename) ..." << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        // file_descriptor_sink supports 64bit offset
        ar::basic_zip_file_sink<io_ex::file_descriptor_sink>
            zip((io_ex::file_descriptor_sink(
                std::string(argv[1]), BOOST_IOS::binary)));

        for (int i = 2; i < argc; ++i)
        {
            ar::zip::header head;
            head.path = fs::path(argv[i]);

            const fs_ex::file_status& s = fs_ex::symlink_status(head.path);

            if (is_symlink(s))
                head.link_path = fs_ex::symlink_target(head.path);
            else if (is_directory(s))
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

            if (s.has_uid() && s.has_gid())
            {
                head.gid = static_cast<boost::uint16_t>(s.gid());
                head.uid = static_cast<boost::uint16_t>(s.uid());
            }

            zip.create_entry(head);

            if (!fs::is_directory(head.path))
            {
                try
                {
                    io::copy(
                        io_ex::file_descriptor_source(
                            head.path.file_string(),
                            std::ios_base::binary),
                        zip
                    );
                }
                catch (const ar::give_up_compression&)
                {
                    zip.rewind_entry();

                    io::copy(
                        io_ex::file_descriptor_source(
                            head.path.file_string(),
                            std::ios_base::binary),
                        zip
                    );
                }
            }
        }
        zip.close_archive();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
