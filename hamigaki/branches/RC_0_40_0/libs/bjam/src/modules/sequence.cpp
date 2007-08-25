// sequence.cpp: bjam sequence module

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/modules/sequence.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/assign/list_of.hpp>
#include <stdexcept>

namespace hamigaki { namespace bjam {

namespace modules
{

namespace sequence
{

HAMIGAKI_BJAM_DECL string_list select_highest_ranked(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& elements = args[0];
    const string_list& rank = args[1];

    if (elements.size() != rank.size())
        throw std::invalid_argument("mismatch sizes of elements and rank");

    const std::size_t size = elements.size();

    int highest = -1;
    for (std::size_t i = 0; i < size; ++i)
        highest = (std::max)(highest, std::atoi(rank[i].c_str()));

    string_list result;
    for (std::size_t i = 0; i < size; ++i)
    {
        if (std::atoi(rank[i].c_str()) == highest)
            result += elements[i];
    }

    return result;
}

} // namespace sequence

HAMIGAKI_BJAM_DECL void set_sequence_rules(context& ctx)
{
    module& m = ctx.get_module(std::string("sequence"));

    {
        native_rule rule;
        rule.parameters.push_back(boost::assign::list_of("elements")("*"));
        rule.parameters.push_back(boost::assign::list_of("rank")("*"));
        rule.native = &sequence::select_highest_ranked;
        rule.version = 1;
        m.native_rules["select-highest-ranked"] = rule;
    }
}

} // namespace modules

} } // End namespaces bjam, hamigaki.
