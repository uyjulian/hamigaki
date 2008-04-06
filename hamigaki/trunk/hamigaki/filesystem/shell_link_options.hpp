// shell_link_options.hpp: the options for Windows Shell links

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_SHELL_LINK_OPTIONS_HPP
#define HAMIGAKI_FILESYSTEM_SHELL_LINK_OPTIONS_HPP

#include <boost/filesystem/config.hpp>
#include <boost/cstdint.hpp>
#include <string>

namespace hamigaki { namespace filesystem {

template<class String>
struct basic_shell_link_options
{
    String description;
    String working_directory;
    String arguments;
    boost::uint16_t hot_key;
    int show_command;
    String icon_path;
    int icon_index;
};

typedef basic_shell_link_options<std::string> shell_link_options;

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
typedef basic_shell_link_options<std::wstring> wshell_link_options;
#endif

} } // End namespaces filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_SHELL_LINK_OPTIONS_HPP
