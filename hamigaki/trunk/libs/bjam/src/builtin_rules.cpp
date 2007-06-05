// builtin_rules.cpp: bjam builtin rules

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#define NOMINMAX
#include <hamigaki/bjam/bjam_context.hpp>
#include <hamigaki/bjam/builtin_rules.hpp>
#include <hamigaki/bjam/bjam_exceptions.hpp>
#include <hamigaki/iterator/ostream_iterator.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace hamigaki { namespace bjam { namespace builtins {

HAMIGAKI_BJAM_DECL list_type echo(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const list_type& arg1 = args[0];

    std::copy(
        arg1.begin(), arg1.end(),
        hamigaki::ostream_iterator<std::string>(std::cout, " ")
    );
    std::cout << '\n';

    return list_type();
}

HAMIGAKI_BJAM_DECL list_type exit(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const list_type& arg1 = args[0];
    const list_type& arg2 = args[1];

    std::ostringstream os;
    std::copy(
        arg1.begin(), arg1.end(),
        hamigaki::ostream_iterator<std::string>(os, " ")
    );

    int code = EXIT_FAILURE;
    if (!arg2.empty())
        code = std::atoi(arg2[0].c_str()); // FIXME

    throw exit_exception(os.str(), code);

    BOOST_UNREACHABLE_RETURN(list_type())
}

} } } // End namespaces builtins, bjam, hamigaki.
