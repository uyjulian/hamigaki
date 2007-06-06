// bjam_expression_actions.hpp: actions for bjam_expression_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_ACTIONS_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_ACTIONS_HPP

#include <hamigaki/bjam/util/list.hpp>
#include <climits> // required for <boost/spirit/phoenix/operators.hpp>
#include <boost/spirit/phoenix.hpp>

namespace hamigaki { namespace bjam {

struct set_true_impl
{
    typedef void result_type;

    void operator()(string_list& lhs, const string_list& rhs) const
    {
        if (!lhs)
        {
            if (rhs)
                lhs = rhs;
            else
                lhs = string_list("1");
        }
    }

    void operator()(string_list& lhs) const
    {
        if (!lhs)
            lhs = string_list("1");
    }
};

const ::phoenix::functor<set_true_impl> set_true = set_true_impl();


struct includes_impl
{
    typedef bool result_type;

    bool operator()(const string_list& lhs, const string_list& rhs) const
    {
        typedef string_list::const_iterator iter_type;

        iter_type lb = lhs.begin();
        iter_type le = lhs.end();
        iter_type rb = rhs.begin();
        iter_type re = rhs.end();

        for (iter_type i = lb; i != le; ++i)
            if (std::find(rb, re, *i) == re)
                return false;

        return true;
    }
};

const ::phoenix::functor<includes_impl> includes = includes_impl();

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_ACTIONS_HPP
