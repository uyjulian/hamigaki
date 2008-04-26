// headers.hpp: tar headers

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_TAR_HEADERS_HPP
#define HAMIGAKI_ARCHIVERS_TAR_HEADERS_HPP

#include <hamigaki/archivers/tar/file_format.hpp>
#include <hamigaki/archivers/tar/raw_header.hpp>
#include <hamigaki/archivers/tar/type_flag.hpp>
#include <hamigaki/filesystem/consts.hpp>
#include <hamigaki/filesystem/timestamp.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <stdexcept>
#include <string>

namespace hamigaki { namespace archivers { namespace tar {

template<class Path>
struct basic_header
{
    typedef Path path_type;
    typedef typename Path::string_type string_type;

    Path path;
    boost::uint16_t permissions;
    boost::intmax_t uid;
    boost::intmax_t gid;
    boost::uintmax_t file_size;
    boost::optional<filesystem::timestamp> modified_time;
    boost::optional<filesystem::timestamp> access_time;
    boost::optional<filesystem::timestamp> change_time;
    char type_flag;
    Path link_path;
    file_format format;
    string_type user_name;
    string_type group_name;
    boost::uint16_t dev_major;
    boost::uint16_t dev_minor;
    string_type comment;
    string_type charset;

    basic_header()
        : permissions(0644), uid(0), gid(0), file_size(0)
        , type_flag(tar::type_flag::regular), format(gnu)
        , dev_major(0), dev_minor(0)
    {
    }

    bool is_regular() const
    {
        return
            (type_flag <= type_flag::regular) ||
            (type_flag >= type_flag::reserved) ;
    }

    bool is_directory() const
    {
        return type_flag == type_flag::directory;
    }

    bool is_symlink() const
    {
        return type_flag == type_flag::symlink;
    }

    bool is_device() const
    {
        return
            (type_flag == type_flag::char_device) ||
            (type_flag == type_flag::block_device) ;
    }

    bool is_long() const
    {
        return
            (type_flag == type_flag::long_link) ||
            (type_flag == type_flag::long_name) ;
    }

    void type(filesystem::file_type v)
    {
        if (v == filesystem::regular_file)
            type_flag = type_flag::regular;
        else if (v == filesystem::directory_file)
            type_flag = type_flag::directory;
        else if (v == filesystem::symlink_file)
            type_flag = type_flag::symlink;
        else if (v == filesystem::block_file)
            type_flag = type_flag::block_device;
        else if (v == filesystem::character_file)
            type_flag = type_flag::char_device;
        else if (v == filesystem::fifo_file)
            type_flag = type_flag::fifo;
        else
            throw std::runtime_error("unsupported file type");
    }
};

typedef basic_header<boost::filesystem::path> header;
#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
typedef basic_header<boost::filesystem::wpath> wheader;
#endif

} } } // End namespaces tar, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_TAR_HEADERS_HPP
