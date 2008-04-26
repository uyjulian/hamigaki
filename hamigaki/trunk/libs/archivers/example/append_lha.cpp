// append_lha.cpp: append files to the existing LZH file

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#include <boost/config.hpp>

#include <hamigaki/archivers/lzh_file.hpp>
#include <hamigaki/filesystem/operations.hpp>
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
            std::cerr
                << "Usage: append_lha (archive) (filename) ..."
                << std::endl;
            return 1;
        }

        std::setlocale(LC_ALL, "");

        io_ex::file_sink file;
        try
        {
            file.open(argv[1], BOOST_IOS::binary|BOOST_IOS::in);
        }
        catch (const BOOST_IOSTREAMS_FAILURE&)
        {
            file.open(argv[1], BOOST_IOS::binary);
        }
        std::streampos pos = io::seek(file, 0, BOOST_IOS::end);
        if (io::position_to_offset(pos) != 0)
            io::seek(file, -1, BOOST_IOS::cur);

        ar::basic_lzh_file_sink<io_ex::file_sink> lzh(file);

        for (int i = 2; i < argc; ++i)
        {
            ar::lha::header head;
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
                        io_ex::file_source(
                            head.path.file_string(),
                            std::ios_base::binary),
                        lzh
                    );
                }
                catch (const ar::give_up_compression&)
                {
                    lzh.rewind_entry();

                    io::copy(
                        io_ex::file_source(
                            head.path.file_string(),
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
