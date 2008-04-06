// config.hpp: configuration file for Hamigaki.Filesystem

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_FILESYSTEM_DETAIL_CONFIG_HPP
#define HAMIGAKI_FILESYSTEM_DETAIL_CONFIG_HPP

#include <boost/filesystem/config.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/system/system_error.hpp>
#include <boost/version.hpp>

#if defined(BOOST_HAS_DECLSPEC)
    #if defined(HAMIGAKI_ALL_DYN_LINK) || defined(HAMIGAKI_FILESYSTEM_DYN_LINK)
        #if defined(HAMIGAKI_FILESYSTEM_SOURCE)
            #define HAMIGAKI_FILESYSTEM_DECL __declspec(dllexport)
        #else
            #define HAMIGAKI_FILESYSTEM_DECL __declspec(dllimport)
        #endif
    #endif
#endif // defined(BOOST_HAS_DECLSPEC)

#if !defined(HAMIGAKI_FILESYSTEM_DECL)
    #define HAMIGAKI_FILESYSTEM_DECL
#endif

namespace hamigaki { namespace filesystem {

typedef boost::system::error_code error_code;

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
typedef boost::filesystem::wpath wpath;
#endif
typedef boost::filesystem::path path;

namespace detail
{

inline error_code make_error_code(int code)
{
    return boost::system::error_code(
        code, boost::system::get_system_category());
}

} // namespace detail

} } // End namespaces filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_DETAIL_CONFIG_HPP
