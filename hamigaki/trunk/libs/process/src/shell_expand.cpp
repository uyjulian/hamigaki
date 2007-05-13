// shell_expand.cpp: shell command expansion

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#define HAMIGAKI_PROCESS_SOURCE
#include <hamigaki/process/shell.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/copy.hpp>

#include "detail/shell_expand_filter.hpp"

namespace io = boost::iostreams;

namespace hamigaki { namespace process {

HAMIGAKI_PROCESS_DECL std::string shell_expand(const std::string& cmd)
{
    context ctx;
    ctx.stdout_behavior(capture_stream());

    child c = launch_shell(cmd, ctx);

    pipe_source& src = c.stdout_source();

    std::string buf;
    io::copy(
        io::compose(detail::shell_expand_filter(), src),
        io::back_inserter(buf)
    );

    c.wait();

    return buf;
}

} } // End namespaces process, hamigaki.
