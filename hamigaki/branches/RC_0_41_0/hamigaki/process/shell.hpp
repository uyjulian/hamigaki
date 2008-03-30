// shell.hpp: utilities for command shell

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_LAUNCH_SHELL_HPP
#define HAMIGAKI_PROCESS_LAUNCH_SHELL_HPP

#include <hamigaki/process/child.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace process {

HAMIGAKI_PROCESS_DECL
child launch_shell(const std::string& cmd, const context& ctx);

inline child launch_shell(const std::string& cmd)
{
    return process::launch_shell(cmd, context());
}

HAMIGAKI_PROCESS_DECL std::string shell_expand(const std::string& cmd);

} } // End namespaces process, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_PROCESS_LAUNCH_SHELL_HPP
