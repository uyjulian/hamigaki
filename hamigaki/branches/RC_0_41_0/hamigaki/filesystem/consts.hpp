// consts.hpp: the constants for the file system

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_CONSTS_HPP
#define HAMIGAKI_FILESYSTEM_CONSTS_HPP

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


struct file_attributes
{
    typedef unsigned long value_type;

    static const value_type type_mask       = 0x00000450ul;

    static const value_type read_only       = 0x00000001ul;
    static const value_type hidden          = 0x00000002ul;
    static const value_type system          = 0x00000004ul;
    static const value_type directory       = 0x00000010ul;
    static const value_type archive         = 0x00000020ul;
    static const value_type device          = 0x00000040ul;
    static const value_type temporary       = 0x00000100ul;
    static const value_type sparse          = 0x00000200ul;
    static const value_type reparse_point   = 0x00000400ul;
    static const value_type compressed      = 0x00000800ul;
    static const value_type offline         = 0x00001000ul;
    static const value_type not_indexed     = 0x00002000ul;
    static const value_type encrypted       = 0x00004000ul;
};


struct file_permissions
{
    typedef unsigned value_type;

    static const value_type type_mask       = 0170000;

    static const value_type socket          = 0140000;
    static const value_type symlink         = 0120000;
    static const value_type regular         = 0100000;
    static const value_type block           = 0060000;
    static const value_type directory       = 0040000;
    static const value_type character       = 0020000;
    static const value_type fifo            = 0010000;

    static const value_type set_uid         = 04000;
    static const value_type set_gid         = 02000;
    static const value_type sticky          = 01000;
    static const value_type user_read       = 00400;
    static const value_type user_write      = 00200;
    static const value_type user_execute    = 00100;
    static const value_type group_read      = 00040;
    static const value_type group_write     = 00020;
    static const value_type group_execute   = 00010;
    static const value_type other_read      = 00004;
    static const value_type other_write     = 00002;
    static const value_type other_execute   = 00001;

    static bool is_regular(value_type v)
    {
        return (v & type_mask) == regular;
    }

    static bool is_directory(value_type v)
    {
        return (v & type_mask) == directory;
    }

    static bool is_symlink(value_type v)
    {
        return (v & type_mask) == symlink;
    }
};

} } // End namespaces filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_CONSTS_HPP
