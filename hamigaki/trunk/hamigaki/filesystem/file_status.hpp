//  file_status.hpp: the file status class

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_FILE_STATUS_HPP
#define HAMIGAKI_FILESYSTEM_FILE_STATUS_HPP

#include <hamigaki/filesystem/timestamp.hpp>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

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


typedef unsigned long file_attributes;

const file_attributes read_only     = 0x00000001ul;
const file_attributes hidden        = 0x00000002ul;
const file_attributes system        = 0x00000004ul;
const file_attributes archive       = 0x00000020ul;
const file_attributes temporary     = 0x00000100ul;
const file_attributes sparse        = 0x00000200ul;
const file_attributes compressed    = 0x00000800ul;
const file_attributes offline       = 0x00001000ul;
const file_attributes not_indexed   = 0x00002000ul;
const file_attributes encrypted     = 0x00004000ul;


typedef unsigned file_permissions;

const file_permissions set_uid          = 04000;
const file_permissions set_gid          = 02000;
const file_permissions sticky           = 01000;
const file_permissions user_read        = 00400;
const file_permissions user_write       = 00200;
const file_permissions user_execute     = 00100;
const file_permissions group_read       = 00040;
const file_permissions group_write      = 00020;
const file_permissions group_execute    = 00010;
const file_permissions other_read       = 00004;
const file_permissions other_write      = 00002;
const file_permissions other_execute    = 00001;


class file_status
{
public:
    explicit file_status(file_type v = status_unknown)
        : type_(v), file_size_(0)
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


    bool has_attributes() const
    {
        return attributes_;
    }

    file_attributes attributes() const
    {
        return *attributes_;
    }

    void attributes(file_attributes v)
    {
        attributes_ = v;
    }


    bool has_permissions() const
    {
        return permissions_;
    }

    file_permissions permissions() const
    {
        return *permissions_;
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
    boost::optional<file_attributes> attributes_;
    boost::optional<file_permissions> permissions_;
    boost::uintmax_t file_size_;
    timestamp last_write_time_;
    timestamp last_access_time_;
    boost::optional<timestamp> last_change_time_;
    boost::optional<timestamp> creation_time_;
    boost::optional<boost::intmax_t> uid_;
    boost::optional<boost::intmax_t> gid_;
};


// predicate functions

inline bool status_known(const file_status& s)
{
    return s.type() != status_unknown;
}

inline bool exists(const file_status& s)
{
    return status_known(s) && (s.type() != file_not_found);
}

inline bool is_regular(const file_status& s)
{
    return s.type() == regular_file;
}

inline bool is_directory(const file_status& s)
{
    return s.type() == directory_file;
}

inline bool is_symlink(const file_status& s)
{
    return s.type() == symlink_file;
}

inline bool is_other(const file_status& s)
{
    return exists(s) && !is_regular(s) && !is_directory(s) && !is_symlink(s);
}


} } // End namespaces filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_FILE_STATUS_HPP
