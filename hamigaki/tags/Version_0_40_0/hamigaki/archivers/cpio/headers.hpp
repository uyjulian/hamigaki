// headers.hpp: cpio headers

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_CPIO_HEADERS_HPP
#define HAMIGAKI_ARCHIVERS_CPIO_HEADERS_HPP

#include <hamigaki/archivers/cpio/binary_header.hpp>
#include <hamigaki/archivers/cpio/file_format.hpp>
#include <hamigaki/archivers/cpio/raw_header.hpp>
#include <hamigaki/archivers/cpio/svr4_header.hpp>
#include <hamigaki/filesystem/consts.hpp>
#include <hamigaki/filesystem/device_number.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <ctime>
#include <stdexcept>
#include <string>

namespace hamigaki { namespace archivers { namespace cpio {

struct header
{
    file_format format;
    filesystem::device_number parent_device;
    boost::uint32_t file_id;
    boost::uint16_t permissions;
    boost::uint32_t uid;
    boost::uint32_t gid;
    boost::uint32_t links;
    filesystem::device_number device;
    std::time_t modified_time;
    boost::filesystem::path path;
    boost::filesystem::path link_path;
    boost::uint32_t file_size;
    boost::optional<boost::uint16_t> checksum;

    header()
        : format(posix), file_id(0), permissions(0100644)
        , uid(0), gid(0), links(1), modified_time(0), file_size(0)
    {
    }

    bool is_regular() const
    {
        return filesystem::file_permissions::is_regular(permissions);
    }

    bool is_directory() const
    {
        return filesystem::file_permissions::is_directory(permissions);
    }

    bool is_symlink() const
    {
        return filesystem::file_permissions::is_symlink(permissions);
    }

    void type(filesystem::file_type v)
    {
        using hamigaki::filesystem::file_permissions;
        file_permissions::value_type mask = ~file_permissions::type_mask;

        if (v == filesystem::regular_file)
            permissions = file_permissions::regular | (permissions & mask);
        else if (v == filesystem::directory_file)
            permissions = file_permissions::directory | (permissions & mask);
        else if (v == filesystem::symlink_file)
            permissions = file_permissions::symlink | (permissions & mask);
        else if (v == filesystem::block_file)
            permissions = file_permissions::block | (permissions & mask);
        else if (v == filesystem::character_file)
            permissions = file_permissions::character | (permissions & mask);
        else if (v == filesystem::fifo_file)
            permissions = file_permissions::fifo | (permissions & mask);
        else if (v == filesystem::socket_file)
            permissions = file_permissions::socket | (permissions & mask);
        else
            throw std::runtime_error("unsupported file type");
    }
};

} } } // End namespaces cpio, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_CPIO_HEADERS_HPP
