// show_status.cpp: show the status of the file

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include <hamigaki/filesystem/operations.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace fs_ex = hamigaki::filesystem;
namespace fs = boost::filesystem;

std::string to_local_time_string(const fs_ex::timestamp& ts)
{
    std::time_t t = ts.to_time_t();
    std::tm lt = *std::localtime(&t);

    std::ostringstream os;
    os
        << std::setfill('0')
        << (1900 + lt.tm_year) << '/'
        << std::setw(2) << (1 + lt.tm_mon) << '/'
        << std::setw(2) << lt.tm_mday << ' '
        << std::setw(2) << lt.tm_hour << ':'
        << std::setw(2) << lt.tm_min << ':'
        << std::setw(2) << lt.tm_sec << '.'
        << std::setw(9) << ts.nanoseconds;
    return os.str();
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: show_status (path)" << std::endl;
            return 1;
        }

        fs::path ph(argv[1]);
        const fs_ex::file_status& s = fs_ex::status(ph);
        if (s.type() == fs_ex::file_not_found)
            throw std::runtime_error("file not found");

        std::cout << "Type:\t\t\t";
        if (s.type() == fs_ex::regular_file)
            std::cout << "regular file";
        else if (s.type() == fs_ex::directory_file)
            std::cout << "directory";
        else if (s.type() == fs_ex::symlink_file)
            std::cout << "symbolic link";
        else if (s.type() == fs_ex::block_file)
            std::cout << "block device";
        else if (s.type() == fs_ex::character_file)
            std::cout << "character device";
        else if (s.type() == fs_ex::fifo_file)
            std::cout << "FIFO";
        else if (s.type() == fs_ex::socket_file)
            std::cout << "socket";
        else
            std::cout << "unknown";
        std::cout << "\n";

        std::cout << "Attributes:\n";
        if ((s.attributes() & fs_ex::file_attributes::read_only) != 0)
            std::cout << "\t\t\tRead only\n";
        if ((s.attributes() & fs_ex::file_attributes::hidden) != 0)
            std::cout << "\t\t\tHidden\n";
        if ((s.attributes() & fs_ex::file_attributes::system) != 0)
            std::cout << "\t\t\tSystem\n";
        if ((s.attributes() & fs_ex::file_attributes::archive) != 0)
            std::cout << "\t\t\tArchive\n";
        if ((s.attributes() & fs_ex::file_attributes::temporary) != 0)
            std::cout << "\t\t\tTemporary\n";
        if ((s.attributes() & fs_ex::file_attributes::sparse) != 0)
            std::cout << "\t\t\tSparse\n";
        if ((s.attributes() & fs_ex::file_attributes::compressed) != 0)
            std::cout << "\t\t\tCompressed\n";
        if ((s.attributes() & fs_ex::file_attributes::offline) != 0)
            std::cout << "\t\t\tOffline\n";
        if ((s.attributes() & fs_ex::file_attributes::not_indexed) != 0)
            std::cout << "\t\t\tNot content indexed\n";
        if ((s.attributes() & fs_ex::file_attributes::encrypted) != 0)
            std::cout << "\t\t\tEncrypted\n";
        std::cout << "\n";

        if (s.has_permissions())
        {
            std::cout
                << "Permissions:\t\t"
                << std::oct << std::showbase
                << s.permissions()
                << std::dec << std::noshowbase
                << "\n";
        }

        std::cout << "Size:\t\t\t" << s.file_size() << "\n";

        std::cout
            << "Last write time:\t"
            << ::to_local_time_string(s.last_write_time())
            << "\n";

        std::cout
            << "Last access time:\t"
            << ::to_local_time_string(s.last_access_time())
            << "\n";

        if (s.has_last_change_time())
        {
            std::cout
                << "Last change time:\t"
                << ::to_local_time_string(s.last_change_time())
                << "\n";
        }

        if (s.has_creation_time())
        {
            std::cout
                << "Creation time:\t\t"
                << ::to_local_time_string(s.creation_time())
                << "\n";
        }

        if (s.has_uid())
            std::cout << "User ID:\t\t" << s.uid() << "\n";

        if (s.has_gid())
            std::cout << "Group ID:\t\t" << s.gid() << "\n";

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
