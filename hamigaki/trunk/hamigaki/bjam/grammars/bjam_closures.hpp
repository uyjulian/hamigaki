// bjam_closures.hpp: closures for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_CLOSURES_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_CLOSURES_HPP

#include <hamigaki/bjam/grammars/assign_modes.hpp>
#include <hamigaki/bjam/util/list.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>

namespace hamigaki { namespace bjam {

struct list_closure
    : boost::spirit::closure<
          list_closure
        , list_type
    >
{
    member1 val;
};

struct lol_closure
    : boost::spirit::closure<
          lol_closure
        , list_of_list
    >
{
    member1 val;
};

struct assign_closure
    : boost::spirit::closure<
          assign_closure
        , assign_mode::values
    >
{
    member1 val;
};

struct set_stmt_closure
    : boost::spirit::closure<
          set_stmt_closure
        , list_type
        , list_type
        , assign_mode::values
    >
{
    member1 val;
    member2 names;
    member3 mode;
};

struct rule_stmt_closure
    : boost::spirit::closure<
          rule_stmt_closure
        , std::string
        , list_of_list
        , bool
    >
{
    member1 name;
    member2 params;
    member3 exported;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_CLOSURES_HPP
