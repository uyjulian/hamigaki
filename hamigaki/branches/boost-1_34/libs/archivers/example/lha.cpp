//  lha.cpp: a simple LZH compressing program

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <boost/config.hpp>

#include <hamigaki/archivers/lzh_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
#include <hamigaki/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/copy.hpp>
#include <clocale>
#include <exception>
#include <iostream>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#endif

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
            std::cerr << "Usage: lha (archive) (filename) ..." << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");
        fs::path::default_name_check(fs::no_check);

        // file_descriptor_sink supports 64bit offset
        ar::basic_lzh_file_sink<io_ex::file_descriptor_sink>
            lzh((io_ex::file_descriptor_sink(
                std::string(argv[1]), BOOST_IOS::binary)));

        for (int i = 2; i < argc; ++i)
        {
            ar::lha::header head;
            head.path = fs::path(argv[i], fs::native);

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
                ar::lha::windows::timestamp ts;
                ts.creation_time = s.creation_time().to_windows_file_time();
                ts.last_write_time = s.last_write_time().to_windows_file_time();
                ts.last_access_time =
                    s.last_access_time().to_windows_file_time();

                head.timestamp = ts;
            }

            if (s.has_permissions())
                head.permissions = s.permissions();

            if (s.has_uid() && s.has_gid())
            {
                ar::lha::posix::gid_uid owner;
                owner.gid = s.gid();
                owner.uid = s.uid();
                head.owner = owner;
            }

#if defined(BOOST_WINDOWS)
            head.code_page = ::GetACP();
#endif

            lzh.create_entry(head);

            if (!fs::is_directory(head.path))
            {
                try
                {
                    io::copy(
                        io_ex::file_descriptor_source(
                            head.path.native_file_string(),
                            std::ios_base::binary),
                        lzh
                    );
                }
                catch (const ar::give_up_compression&)
                {
                    lzh.rewind_entry();

                    io::copy(
                        io_ex::file_descriptor_source(
                            head.path.native_file_string(),
                            std::ios_base::binary),
                        lzh
                    );
                }
            }
        }
        lzh.close_archive();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
