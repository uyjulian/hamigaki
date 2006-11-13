//  operations.hpp: the file operations

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_OPERATIONS_HPP
#define HAMIGAKI_FILESYSTEM_OPERATIONS_HPP

#include <hamigaki/filesystem/detail/config.hpp>
#include <hamigaki/filesystem/detail/auto_link.hpp>
#include <hamigaki/filesystem/file_status.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/path.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace filesystem {

// status functions

HAMIGAKI_FILESYSTEM_DECL file_status
status(const boost::filesystem::path& p, int& ec);

inline file_status status(const boost::filesystem::path& p)
{
    int ec;
    const file_status& s = filesystem::status(p, ec);
    if (ec != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::status", p, ec);
    }
    return s;
}


HAMIGAKI_FILESYSTEM_DECL file_status
symlink_status(const boost::filesystem::path& p, int& ec);

inline file_status symlink_status(const boost::filesystem::path& p)
{
    int ec;
    const file_status& s = filesystem::symlink_status(p, ec);
    if (ec != 0)
    {
        throw boost::filesystem::filesystem_error(
            "hamigaki::filesystem::symlink_status", p, ec);
    }
    return s;
}


// predicate functions

inline bool exists(const boost::filesystem::path& p)
{
    return exists(filesystem::status(p));
}

inline bool is_directory(const boost::filesystem::path& p)
{
    return is_directory(filesystem::status(p));
}

inline bool is_regular(const boost::filesystem::path& p)
{
    return is_regular(filesystem::status(p));
}

inline bool is_other(const boost::filesystem::path& p)
{
    return is_other(filesystem::status(p));
}

inline bool is_symlink(const boost::filesystem::path& p)
{
    return is_symlink(filesystem::symlink_status(p));
}


// attribute functions

HAMIGAKI_FILESYSTEM_DECL
boost::filesystem::path symlink_target(const boost::filesystem::path& p);

} } // End namespaces filesystem, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_FILESYSTEM_OPERATIONS_HPP
