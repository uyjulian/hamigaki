//  headers.hpp: ZIP headers

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_HEADERS_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_HEADERS_HPP

#include <hamigaki/archivers/msdos/attributes.hpp>
#include <hamigaki/archivers/zip/archive_extra_data.hpp>
#include <hamigaki/archivers/zip/data_descriptor.hpp>
#include <hamigaki/archivers/zip/digital_signature.hpp>
#include <hamigaki/archivers/zip/end_of_central_directory.hpp>
#include <hamigaki/archivers/zip/extra_field_header.hpp>
#include <hamigaki/archivers/zip/file_header.hpp>
#include <hamigaki/archivers/zip/local_file_header.hpp>
#include <hamigaki/archivers/zip/zip64_end_cent_dir.hpp>
#include <hamigaki/archivers/zip/zip64_end_cent_dir_locator.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <string>

namespace hamigaki { namespace archivers { namespace zip {

struct consts
{
    static const std::size_t encryption_header_size = 12;
};

struct flags
{
    static const boost::uint16_t encrypted      = 0x0001;
    static const boost::uint16_t has_data_dec   = 0x0008;
};

struct method
{
    static const boost::uint16_t store      = 0;
    static const boost::uint16_t deflate    = 8;
    static const boost::uint16_t bzip2      = 12;
};

struct internal_attributes
{
    static const boost::uint16_t ascii  = 0x0001;
};

struct extra_field_id
{
    static const boost::uint16_t zip64              = 0x0001;
    static const boost::uint16_t extended_timestamp = 0x5455;
    static const boost::uint16_t info_zip_unix2     = 0x7855;
};

struct header
{
    boost::filesystem::path path;
    boost::filesystem::path link_path;
    boost::uint8_t version;
    bool encrypted;
    boost::uint16_t encryption_checksum;
    boost::uint16_t method;
    std::time_t update_time;
    boost::uint32_t crc32_checksum;
    boost::uint64_t compressed_size;
    boost::uint64_t file_size;
    boost::uint16_t attributes;
    boost::uint16_t permissions;
    std::string comment;

    boost::optional<std::time_t> modified_time;
    boost::optional<std::time_t> access_time;
    boost::optional<std::time_t> creation_time;

    boost::optional<boost::uint16_t> uid;
    boost::optional<boost::uint16_t> gid;

    header()
        : version(20), encrypted(false), encryption_checksum(0)
        , method(zip::method::deflate), update_time(0), crc32_checksum(0)
        , compressed_size(0), file_size(0)
        , attributes(msdos::attributes::archive), permissions(0644)
    {
    }

    bool is_regular() const
    {
        return !is_symlink() && !is_directory();
    }

    bool is_directory() const
    {
        return (attributes & msdos::attributes::directory) != 0;
    }

    bool is_symlink() const
    {
        return !link_path.empty();
    }

    bool match_encryption_checksum(boost::uint16_t value) const
    {
        if (version < 20)
            return value == encryption_checksum;
        else
            return (value >> 8) == (encryption_checksum >> 8);
    }
};

class header_path_match
{
public:
    explicit header_path_match(const boost::filesystem::path& ph) : path_(ph)
    {
    }

    bool operator()(const header& head) const
    {
        return head.path == path_;
    }

private:
    boost::filesystem::path path_;
};

} } } // End namespaces zip, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ZIP_HEADERS_HPP
