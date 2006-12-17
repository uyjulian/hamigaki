//  headers.hpp: tar headers

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_TAR_HEADERS_HPP
#define HAMIGAKI_ARCHIVERS_TAR_HEADERS_HPP

#include <hamigaki/archivers/tar/file_format.hpp>
#include <hamigaki/archivers/tar/raw_header.hpp>
#include <hamigaki/archivers/tar/type.hpp>
#include <hamigaki/filesystem/timestamp.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <string>

namespace hamigaki { namespace archivers { namespace tar {

struct header
{
    boost::filesystem::path path;
    boost::uint16_t permissions;
    boost::intmax_t uid;
    boost::intmax_t gid;
    boost::uintmax_t file_size;
    boost::optional<filesystem::timestamp> modified_time;
    boost::optional<filesystem::timestamp> access_time;
    boost::optional<filesystem::timestamp> change_time;
    char type;
    boost::filesystem::path link_path;
    file_format format;
    std::string user_name;
    std::string group_name;
    boost::uint16_t dev_major;
    boost::uint16_t dev_minor;
    std::string comment;

    header()
        : permissions(0644), uid(0), gid(0), file_size(0)
        , type(tar::type::regular), format(gnu), dev_major(0), dev_minor(0)
    {
    }

    bool is_regular() const
    {
        return (type <= type::regular) || (type >= type::reserved);
    }

    bool is_device() const
    {
        return (type == type::char_device) || (type == type::block_device);
    }

    bool is_long() const
    {
        return (type == type::long_link) || (type == type::long_name);
    }
};

} } } // End namespaces tar, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_TAR_HEADERS_HPP
