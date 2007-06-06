// bjam_closures.hpp: closures for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_CLOSURES_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_CLOSURES_HPP

#include <hamigaki/bjam/grammars/assign_modes.hpp>
#include <hamigaki/bjam/util/list_of_list.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/optional.hpp>

namespace hamigaki { namespace bjam {

struct invoke_stmt_closure
    : boost::spirit::closure<
          invoke_stmt_closure
        , string_list
        , boost::optional<std::string>
        , list_of_list
    >
{
    member1 values;
    member2 name;
    member3 args;
};

struct assign_closure
    : boost::spirit::closure<
          assign_closure
        , assign_mode::values
    >
{
    member1 values;
};

struct set_stmt_closure
    : boost::spirit::closure<
          set_stmt_closure
        , string_list
        , string_list
        , assign_mode::values
    >
{
    member1 values;
    member2 names;
    member3 mode;
};

struct for_stmt_closure
    : boost::spirit::closure<
          for_stmt_closure
        , std::string
        , string_list
        , bool
    >
{
    member1 name;
    member2 values;
    member3 is_local;
};

struct switch_stmt_closure
    : boost::spirit::closure<
          switch_stmt_closure
        , string_list
        , std::string
        , std::string
    >
{
    member1 values;
    member2 value;
    member3 pattern;
};

struct module_stmt_closure
    : boost::spirit::closure<
          module_stmt_closure
        , string_list
        , boost::optional<std::string>
    >
{
    member1 values;
    member2 name;
};

struct while_stmt_closure
    : boost::spirit::closure<
          while_stmt_closure
        , string_list
        , std::string
    >
{
    member1 values;
    member2 expr;
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
