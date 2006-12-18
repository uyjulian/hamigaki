//  headers.hpp: LZH headers

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_HEADERS_HPP
#define HAMIGAKI_ARCHIVERS_LHA_HEADERS_HPP

#include <hamigaki/archivers/lha/posix/gid_uid.hpp>
#include <hamigaki/archivers/lha/windows/file_size.hpp>
#include <hamigaki/archivers/lha/windows/timestamp.hpp>
#include <hamigaki/archivers/lha/lv0_header.hpp>
#include <hamigaki/archivers/lha/lv1_header.hpp>
#include <hamigaki/archivers/lha/lv2_header.hpp>
#include <hamigaki/archivers/msdos/attributes.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

namespace hamigaki { namespace archivers { namespace lha {

struct header
{
    boost::uint8_t level;
    compress_method method;
    boost::int64_t compressed_size;
    boost::int64_t file_size;
    std::time_t update_time;
    boost::uint16_t attributes;
    boost::filesystem::path path;
    boost::filesystem::path link_path;
    boost::optional<boost::uint16_t> crc16_checksum;
    boost::optional<char> os;
    boost::optional<windows::timestamp> timestamp;
    boost::optional<boost::uint32_t> code_page;
    boost::optional<boost::uint16_t> permissions;
    boost::optional<posix::gid_uid> owner;
    std::string group_name;
    std::string user_name;
    std::string comment;

    header()
        : level(2), compressed_size(-1), file_size(-1), update_time(-1)
        , attributes(msdos::attributes::archive)
    {
    }

    bool is_directory() const
    {
        return (attributes & msdos::attributes::directory) != 0;
    }

    bool is_symbolic_link() const
    {
        return !link_path.empty();
    }
};

} } } // End namespaces lha, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_HEADERS_HPP
