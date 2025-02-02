// regex.cpp: bjam regex module

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM2_SOURCE
#include <hamigaki/bjam2/modules/regex.hpp>
#include <hamigaki/bjam2/util/regex.hpp>
#include <hamigaki/bjam2/bjam_context.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/regex.hpp>
#include <vector>

namespace hamigaki { namespace bjam2 {

namespace modules
{

namespace regex
{

HAMIGAKI_BJAM2_DECL string_list transform(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& list = args[0];
    const std::string& pattern = bjam2::convert_regex(args[1][0]);
    const string_list& arg3 = args[2];

    std::vector<int> indices;
    if (arg3.empty())
        indices.push_back(1);
    else
    {
        indices.reserve(arg3.size());
        for (std::size_t i = 0, size = arg3.size(); i < size; ++i)
            indices.push_back(std::atoi(arg3[i].c_str()));
    }

    string_list result;

    // Note: bjam's regex is not the same as "egrep" and "ECMAScript"
    boost::basic_regex<char,boost::c_regex_traits<char> > rex(pattern);
    for (std::size_t i = 0, lsize = list.size(); i < lsize; ++i)
    {
        boost::smatch what;
        if (regex_search(list[i], what, rex))
        {
            for (std::size_t j = 0, isize = indices.size(); j < isize; ++j)
            {
                const std::string& s = what[indices[j]].str();
                if (!s.empty())
                    result += s;
            }
        }
    }

    return result;
}

} // namespace regex

HAMIGAKI_BJAM2_DECL void set_regex_rules(context& ctx)
{
    module& m = ctx.get_module(std::string("regex"));

    {
        native_rule rule;
        rule.parameters.push_back(boost::assign::list_of("list")("*"));
        rule.parameters.push_back(boost::assign::list_of("pattern"));
        rule.parameters.push_back(boost::assign::list_of("indices")("*"));
        rule.native = &regex::transform;
        rule.version = 2;
        m.native_rules["transform"] = rule;
    }
}

} // namespace modules

} } // End namespaces bjam2, hamigaki.
