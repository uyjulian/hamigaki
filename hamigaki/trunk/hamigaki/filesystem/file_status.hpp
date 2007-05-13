// file_status.hpp: the file status class

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_FILE_STATUS_HPP
#define HAMIGAKI_FILESYSTEM_FILE_STATUS_HPP

#include <hamigaki/filesystem/consts.hpp>
#include <hamigaki/filesystem/timestamp.hpp>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>

namespace hamigaki { namespace filesystem {

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

    file_attributes::value_type attributes() const
    {
        file_attributes::value_type tmp = *attributes_;

        if (type_ == directory_file)
            tmp |= file_attributes::directory;
        else if (type_ == symlink_file)
            tmp |= file_attributes::reparse_point;

        return tmp;
    }

    void attributes(file_attributes::value_type v)
    {
        attributes_ = v & ~file_attributes::type_mask;
    }


    bool has_permissions() const
    {
        return permissions_;
    }

    file_permissions::value_type permissions() const
    {
        file_permissions::value_type tmp = *permissions_;

        if (type_ == regular_file)
            tmp |= file_permissions::regular;
        else if (type_ == directory_file)
            tmp |= file_permissions::directory;
        else if (type_ == symlink_file)
            tmp |= file_permissions::symlink;
        else if (type_ == block_file)
            tmp |= file_permissions::block;
        else if (type_ == character_file)
            tmp |= file_permissions::character;
        else if (type_ == fifo_file)
            tmp |= file_permissions::fifo;
        else if (type_ == socket_file)
            tmp |= file_permissions::socket;

        return tmp;
    }

    void permissions(file_permissions::value_type v)
    {
        permissions_ = v & ~file_permissions::type_mask;;
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
    boost::optional<file_attributes::value_type> attributes_;
    boost::optional<file_permissions::value_type> permissions_;
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
