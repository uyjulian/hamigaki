// headers.hpp: ZIP headers

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

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
#include <hamigaki/filesystem/consts.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <stdexcept>
#include <string>

namespace hamigaki { namespace archivers { namespace zip {

struct consts
{
    static const std::size_t encryption_header_size = 12;
};

struct os
{
    static const boost::uint8_t ms_dos     = 0x00; // NTFS for PKZIP
    static const boost::uint8_t posix      = 0x03; // includes Mac OS X
    static const boost::uint8_t winnt      = 0x0B; // used by Info-ZIP, MS
};

struct flags
{
    static const boost::uint16_t encrypted      = 0x0001;
    static const boost::uint16_t has_data_dec   = 0x0008;
    static const boost::uint16_t utf8_encoded   = 0x0800;
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

template<class Path>
struct basic_header
{
    typedef Path path_type;
    typedef typename Path::string_type string_type;

    Path path;
    Path link_path;
    boost::uint8_t os;
    boost::uint8_t version;
    bool encrypted;
    bool utf8_encoded;
    boost::uint16_t encryption_checksum;
    boost::uint16_t method;
    std::time_t update_time;
    boost::uint32_t crc32_checksum;
    boost::uint64_t compressed_size;
    boost::uint64_t file_size;
    boost::uint16_t attributes;
    boost::uint16_t permissions;
    string_type comment;

    boost::optional<std::time_t> modified_time;
    boost::optional<std::time_t> access_time;
    boost::optional<std::time_t> creation_time;

    boost::optional<boost::uint16_t> uid;
    boost::optional<boost::uint16_t> gid;

    basic_header()
        : os(0), version(20), encrypted(false), utf8_encoded(false)
        , encryption_checksum(0)
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

    void type(filesystem::file_type v)
    {
        using hamigaki::filesystem::file_permissions;
        file_permissions::value_type mask = ~file_permissions::type_mask;

        if (v == filesystem::regular_file)
        {
            attributes &= ~msdos::attributes::directory;
            permissions = file_permissions::regular | (permissions & mask);
        }
        else if (v == filesystem::directory_file)
        {
            attributes |= msdos::attributes::directory;
            permissions = file_permissions::directory | (permissions & mask);
        }
        else if (v == filesystem::symlink_file)
        {
            attributes &= ~msdos::attributes::directory;
            permissions = file_permissions::symlink | (permissions & mask);
        }
        else
            throw std::runtime_error("unsupported file type");
    }
};

typedef basic_header<boost::filesystem::path> header;
#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
typedef basic_header<boost::filesystem::wpath> wheader;
#endif

template<class Path>
class basic_header_path_match
{
public:
    explicit basic_header_path_match(const Path& ph) : path_(ph)
    {
    }

    bool operator()(const basic_header<Path>& head) const
    {
        return head.path == path_;
    }

private:
    Path path_;
};

template<class Path>
inline basic_header_path_match<Path> header_path_match(const Path& ph)
{
    return basic_header_path_match<Path>(ph);
}

} } } // End namespaces zip, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ZIP_HEADERS_HPP
