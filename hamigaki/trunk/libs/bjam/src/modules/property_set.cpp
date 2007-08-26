// property_set.cpp: bjam property-set module

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/modules/property_set.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/assign/list_of.hpp>

#if BOOST_VERSION >= 103500
    #include <boost/range/as_literal.hpp>
#endif
#include <boost/algorithm/string/join.hpp>

namespace assign = boost::assign;

namespace hamigaki { namespace bjam {

namespace modules
{

namespace property_set
{

HAMIGAKI_BJAM_DECL string_list create(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    string_list raw = args[0];
    raw.sort();
    raw.unique();

    namespace algo = boost::algorithm;
    const std::string& key = ".ps." + algo::join(raw, "-");

    module& m = f.current_module();
    string_list values = m.variables.get_values(key);
    if (values.empty())
    {
        string_list arg1(std::string("property-set"));
        arg1 += raw;

        list_of_list args;
        args.push_back(arg1);

        values = ctx.invoke_rule("new", args);
        m.variables.set_values(key, values);
    }
    return values;
}

} // namespace property_set

HAMIGAKI_BJAM_DECL void set_property_set_rules(context& ctx)
{
    module& m = ctx.get_module(std::string("property-set"));

    {
        native_rule rule;
        rule.parameters.push_back(assign::list_of("raw-properties")("*"));
        rule.native = &property_set::create;
        rule.version = 1;
        m.native_rules["create"] = rule;
    }
}

} // namespace modules

} } // End namespaces bjam, hamigaki.
