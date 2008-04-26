// path.cpp: bjam path module

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/modules/path.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

namespace hamigaki { namespace bjam {

namespace modules
{

namespace path
{

HAMIGAKI_BJAM_DECL string_list exists(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const std::string& location = args[0][0];

    fs::path ph(location);
    fs::path work(ctx.working_directory());
    ph = fs::complete(ph, work);
    if (fs::exists(ph))
        return string_list(std::string("true"));
    else
        return string_list();
}

} // namespace path

HAMIGAKI_BJAM_DECL void set_path_rules(context& ctx)
{
    module& m = ctx.get_module(std::string("path"));

    {
        native_rule rule;
        rule.parameters.push_back(boost::assign::list_of("location"));
        rule.native = &path::exists;
        rule.version = 1;
        m.native_rules["exists"] = rule;
    }
}

} // namespace modules

} } // End namespaces bjam, hamigaki.
