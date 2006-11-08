//  file_status.hpp: the file status class

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_FILE_STATUS_HPP
#define HAMIGAKI_FILESYSTEM_FILE_STATUS_HPP

#include <hamigaki/filesystem/detail/config.hpp>
#include <hamigaki/filesystem/detail/auto_link.hpp>
#include <hamigaki/filesystem/timestamp.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace filesystem {

enum file_type
{
    status_unknown,
    file_not_found,
    regular_file,
    directory_file,
    symlink_file,
    block_file,
    character_file,
    fifo_file,
    socket_file,
    type_unknown
};


typedef unsigned file_attributes;

const file_attributes set_uid       = (1u <<  0);
const file_attributes set_gid       = (1u <<  1);
const file_attributes sticky        = (1u <<  2);
const file_attributes hidden        = (1u <<  3);
const file_attributes for_system    = (1u <<  4);
const file_attributes for_archive   = (1u <<  5);
const file_attributes temporary     = (1u <<  6);
const file_attributes sparse        = (1u <<  7);
const file_attributes compressed    = (1u <<  8);
const file_attributes offline       = (1u <<  9);
const file_attributes not_indexed   = (1u << 10);
const file_attributes encrypted     = (1u << 11);


typedef unsigned file_permissions;

const file_permissions user_read        = 0400;
const file_permissions user_write       = 0200;
const file_permissions user_execute     = 0100;
const file_permissions group_read       = 0040;
const file_permissions group_write      = 0020;
const file_permissions group_execute    = 0010;
const file_permissions other_read       = 0004;
const file_permissions other_write      = 0002;
const file_permissions other_execute    = 0001;


class file_status
{
public:
    explicit file_status(file_type v = status_unknown)
        : type_(v), attributes_(for_archive), permissions_(0644), file_size_(0)
    {
    }


    file_type type() const
    {
        return type_;
    }

    void type(file_type v)
    {
        type_ = v;
    }


    file_attributes attributes() const
    {
        return attributes_;
    }

    void attributes(file_attributes v)
    {
        attributes_ = v;
    }


    file_permissions permissions() const
    {
        return permissions_;
    }

    void permissions(file_permissions v)
    {
        permissions_ = v;
    }


    boost::uintmax_t file_size() const
    {
        return file_size_;
    }

    void file_size(boost::uintmax_t v)
    {
        file_size_ = v;
    }


    timestamp last_write_time() const
    {
        return last_write_time_;
    }

    void last_write_time(const timestamp& v)
    {
        last_write_time_ = v;
    }


    timestamp last_access_time() const
    {
        return last_access_time_;
    }

    void last_access_time(const timestamp& v)
    {
        last_access_time_ = v;
    }


    bool has_last_change_time() const
    {
        return last_change_time_;
    }

    timestamp last_change_time() const
    {
        return *last_change_time_;
    }

    void last_change_time(const timestamp& v)
    {
        last_change_time_ = v;
    }


    bool has_creation_time() const
    {
        return creation_time_;
    }

    timestamp creation_time() const
    {
        return *creation_time_;
    }

    void creation_time(const timestamp& v)
    {
        creation_time_ = v;
    }


    bool has_uid() const
    {
        return uid_;
    }

    boost::intmax_t uid() const
    {
        return *uid_;
    }

    void uid(boost::intmax_t v)
    {
        uid_ = v;
    }


    bool has_gid() const
    {
        return gid_;
    }

    boost::intmax_t gid() const
    {
        return *gid_;
    }

    void gid(boost::intmax_t v)
    {
        gid_ = v;
    }

private:
    file_type type_;
    file_attributes attributes_;
    file_permissions permissions_;
    boost::uintmax_t file_size_;
    timestamp last_write_time_;
    timestamp last_access_time_;
    boost::optional<timestamp> last_change_time_;
    boost::optional<timestamp> creation_time_;
    boost::optional<boost::intmax_t> uid_;
    boost::optional<boost::intmax_t> gid_;
};

HAMIGAKI_FILESYSTEM_DECL file_status
status(const boost::filesystem::path& p, int& ec);

inline file_status status(const boost::filesystem::path& p)
{
    int ec;
    const file_status& s = status(p, ec);
    if (ec != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::status", p, ec);
    }
    return s;
}

} } // End namespaces filesystem, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_FILESYSTEM_FILE_STATUS_HPP
