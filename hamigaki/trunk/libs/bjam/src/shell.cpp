// shell.cpp: shell utility

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/util/shell.hpp>
#include <hamigaki/process/shell.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <locale>
#include <sstream>

namespace hamigaki { namespace bjam {

HAMIGAKI_BJAM_DECL string_list
shell(const std::string& cmd, bool need_status, bool need_capture)
{
    process::context ctx;
    if (need_capture)
        ctx.stdout_behavior(process::capture_stream());
    else
        ctx.stdout_behavior(process::silence_stream());

    process::child c = process::launch_shell(cmd, ctx);

    std::string buf;
    if (need_capture)
    {
        boost::iostreams::copy(
            c.stdout_source(),
            boost::iostreams::back_inserter(buf)
        );
    }
    process::status stat = c.wait();

    string_list result(buf);
    if (need_status)
    {
        if (stat.get_type() == process::status::exited)
        {
            std::ostringstream os;
            os.imbue(std::locale::classic());
            os << stat.code();
            result += os.str();
        }
        else
            result += std::string("-1");
    }
    return result;
}

} } // End namespaces bjam, hamigaki.
