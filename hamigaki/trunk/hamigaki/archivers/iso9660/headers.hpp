//  headers.hpp: ISO 9660 headers

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO9660_HEADERS_HPP
#define HAMIGAKI_ARCHIVERS_ISO9660_HEADERS_HPP

#include <hamigaki/archivers/iso9660/posix/file_attributes.hpp>
#include <hamigaki/archivers/iso9660/date_time.hpp>
#include <hamigaki/archivers/iso9660/directory_record.hpp>
#include <hamigaki/archivers/iso9660/file_flags.hpp>
#include <hamigaki/archivers/iso9660/path_table_record.hpp>
#include <hamigaki/archivers/iso9660/system_use_entries.hpp>
#include <hamigaki/archivers/iso9660/tf_flags.hpp>
#include <hamigaki/archivers/iso9660/volume_desc_set_terminator.hpp>
#include <hamigaki/archivers/iso9660/volume_descriptor.hpp>
#include <hamigaki/filesystem/consts.hpp>
#include <boost/filesystem/path.hpp>
#include <stdexcept>

namespace hamigaki { namespace archivers { namespace iso9660 {

struct header
{
    boost::filesystem::path path;
    boost::uint32_t file_size;
    binary_date_time recorded_time;
    boost::uint8_t flags;
    std::string system_use;
    boost::optional<posix::file_attributes> attributes;
    date_time creation_time;
    date_time last_write_time;
    date_time last_access_time;
    date_time last_change_time;
    date_time last_backup_time;
    date_time expiration_time;
    date_time effective_time;

    header() : file_size(0), flags(0)
    {
    }

    bool is_regular() const
    {
        return (flags & (file_flags::directory|file_flags::associated)) == 0;
    }

    bool is_directory() const
    {
        return (flags & file_flags::directory) != 0;
    }

    bool is_symlink() const
    {
        return false;
    }

    bool is_associated() const
    {
        return (flags & file_flags::associated) != 0;
    }

    void type(filesystem::file_type v)
    {
        if (v == filesystem::regular_file)
            flags &= ~file_flags::directory;
        else if (v == filesystem::directory_file)
            flags |= file_flags::directory;
        else
            throw std::runtime_error("unsupported file type");
    }
};

} } } // End namespaces iso9660, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO9660_HEADERS_HPP
