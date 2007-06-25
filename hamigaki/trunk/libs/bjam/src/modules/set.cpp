// set.cpp: bjam set module

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/modules/set.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/assign/list_of.hpp>

namespace hamigaki { namespace bjam {

namespace modules
{

namespace set
{

HAMIGAKI_BJAM_DECL string_list difference(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& b = args[0];
    const string_list& a = args[1];

    string_list result;

    for (std::size_t i = 0, size = b.size(); i < size; ++i)
    {
        if (std::find(a.begin(), a.end(), b[i]) == a.end())
            result += b[i];
    }

    return result;
}

} // namespace set

HAMIGAKI_BJAM_DECL void set_set_rules(context& ctx)
{
    module& m = ctx.get_module(std::string("set"));

    {
        native_rule rule;
        rule.parameters.push_back(boost::assign::list_of("B")("*"));
        rule.parameters.push_back(boost::assign::list_of("A")("*"));
        rule.native = &set::difference;
        rule.version = 1;
        m.native_rules["difference"] = rule;
    }
}

} // namespace modules

} } // End namespaces bjam, hamigaki.
