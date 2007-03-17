//  headers.hpp: ISO 9660 headers

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_HEADERS_HPP
#define HAMIGAKI_ARCHIVERS_ISO_HEADERS_HPP

#include <hamigaki/archivers/iso/posix/file_attributes.hpp>
#include <hamigaki/archivers/iso/date_time.hpp>
#include <hamigaki/archivers/iso/directory_record.hpp>
#include <hamigaki/archivers/iso/file_flags.hpp>
#include <hamigaki/archivers/iso/path_table_record.hpp>
#include <hamigaki/archivers/iso/rrip_type.hpp>
#include <hamigaki/archivers/iso/system_use_entries.hpp>
#include <hamigaki/archivers/iso/volume_desc_set_terminator.hpp>
#include <hamigaki/archivers/iso/volume_descriptor.hpp>
#include <hamigaki/filesystem/consts.hpp>
#include <hamigaki/filesystem/device_number.hpp>
#include <boost/filesystem/path.hpp>
#include <stdexcept>

namespace hamigaki { namespace archivers { namespace iso {

struct header
{
    boost::filesystem::path path;
    boost::uint16_t version;
    boost::filesystem::path link_path;
    boost::uint32_t data_pos;
    boost::uint64_t file_size;
    binary_date_time recorded_time;
    boost::uint8_t flags;
    std::string system_use;
    boost::optional<posix::file_attributes> attributes;
    boost::optional<filesystem::device_number> device_number;
    date_time creation_time;
    date_time last_write_time;
    date_time last_access_time;
    date_time last_change_time;
    date_time last_backup_time;
    date_time expiration_time;
    date_time effective_time;

    header() : version(0), data_pos(0), file_size(0), flags(0)
    {
    }

    bool is_regular() const
    {
        if (is_symlink())
            return false;

        return (flags & (file_flags::directory|file_flags::associated)) == 0;
    }

    bool is_directory() const
    {
        return (flags & file_flags::directory) != 0;
    }

    bool is_symlink() const
    {
        return !link_path.empty();
    }

    bool is_associated() const
    {
        return (flags & file_flags::associated) != 0;
    }

    void type(filesystem::file_type v)
    {
        using hamigaki::filesystem::file_permissions;

        if (v == filesystem::regular_file)
            flags &= ~file_flags::directory;
        else if (v == filesystem::directory_file)
        {
            flags |= file_flags::directory;
            set_file_mode(file_permissions::directory);
        }
        else if (v == filesystem::symlink_file)
            set_file_mode(file_permissions::symlink);
        else
            throw std::runtime_error("unsupported file type");
    }

private:
    void set_file_mode(boost::uint16_t type)
    {
        if (!attributes)
        {
            posix::file_attributes attr;
            attr.permissions = 0444;
            attr.links = 1;
            attr.uid = 0;
            attr.gid = 0;
            attr.serial_no = 0;
            attributes = attr;
        }

        boost::uint16_t mask =
            static_cast<boost::uint16_t>(
                ~filesystem::file_permissions::type_mask);
        attributes->permissions = type | (attributes->permissions & mask);
    }
};

struct volume_info
{
    char std_id[5];
    boost::uint32_t volume_space_size;
    boost::uint16_t volume_set_size;
    boost::uint16_t volume_seq_number;
    boost::uint16_t logical_block_size;
    date_time creation_time;
    date_time modification_time;
    date_time expiration_time;
    date_time effective_time;

    volume_info()
        : volume_space_size(0u), volume_set_size(1u)
        , volume_seq_number(1u), logical_block_size(2048)
    {
        std::memcpy(std_id, "CD001", 5);
    }
};

struct volume_desc
{
    unsigned level;
    rrip_type rrip;
    boost::uint8_t type;
    boost::uint8_t version;
    boost::uint8_t flags;
    std::string system_id;
    std::string volume_id;
    std::string escape_sequences;
    boost::uint32_t path_table_size;
    boost::uint32_t l_path_table_pos;
    boost::uint32_t l_path_table_pos2;
    boost::uint32_t m_path_table_pos;
    boost::uint32_t m_path_table_pos2;
    directory_record root_record;
    std::string volume_set_id;
    std::string publisher_id;
    std::string data_preparer_id;
    std::string application_id;
    boost::filesystem::path copyright_file_id;
    boost::filesystem::path abstract_file_id;
    boost::filesystem::path bibliographic_file_id;
    boost::uint8_t file_structure_version;
    char application_use[512];

    volume_desc()
        : level(1u), rrip(rrip_none), type(1u)
        , version(1u), flags(0u), path_table_size(0)
        , l_path_table_pos(0), l_path_table_pos2(0)
        , m_path_table_pos(0), m_path_table_pos2(0)
        , file_structure_version(1u)
    {
        std::memset(application_use, 0, sizeof(application_use));
    }

    bool is_joliet() const
    {
        if (type != 2u)
            return false;

        return
            (escape_sequences == "%/@") ||
            (escape_sequences == "%/C") ||
            (escape_sequences == "%/E") ;
    }

    void set_joliet()
    {
        type = 2u;
        escape_sequences = "%/@";
    }

    bool is_rock_ridge() const
    {
        return rrip != rrip_none;
    }

    void set_enhanced()
    {
        level = 4u;
        type = 2u;
        version = 2u;
        file_structure_version = 2u;
    }

    bool is_enhanced() const
    {
        return version == 2u;
    }
};

} } } // End namespaces iso, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ISO_HEADERS_HPP
