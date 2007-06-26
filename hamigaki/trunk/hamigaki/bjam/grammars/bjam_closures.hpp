// bjam_closures.hpp: closures for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_CLOSURES_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_CLOSURES_HPP

#include <hamigaki/bjam/util/action_modifiers.hpp>
#include <hamigaki/bjam/util/list_of_list.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/optional.hpp>

namespace hamigaki { namespace bjam {

struct local_set_stmt_closure
    : boost::spirit::closure<
          local_set_stmt_closure
        , string_list
        , string_list
    >
{
    member1 values;
    member2 names;
};

struct invoke_stmt_closure
    : boost::spirit::closure<
          invoke_stmt_closure
        , string_list
        , boost::optional<std::string>
        , list_of_list
        , int
    >
{
    member1 values;
    member2 name;
    member3 args;
    member4 caller_line;
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

struct set_on_stmt_closure
    : boost::spirit::closure<
          set_on_stmt_closure
        , string_list
        , string_list
        , assign_mode::values
        , string_list
    >
{
    member1 values;
    member2 names;
    member3 mode;
    member4 targets;
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
    >
{
    member1 values;
    member2 value;
};

struct cases_closure
    : boost::spirit::closure<
          cases_closure
        , string_list
        , std::string
    >
{
    member1 values;
    member2 pattern;
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

struct class_stmt_closure
    : boost::spirit::closure<
          class_stmt_closure
        , std::string
    >
{
    member1 module_name;
};

struct while_stmt_closure
    : boost::spirit::closure<
          while_stmt_closure
        , string_list
        , std::string
        , int
    >
{
    member1 values;
    member2 expr;
    member3 expr_line;
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

struct on_stmt_closure
    : boost::spirit::closure<
          on_stmt_closure
        , string_list
        , string_list
    >
{
    member1 values;
    member2 targets;
};

struct actions_stmt_closure
    : boost::spirit::closure<
          actions_stmt_closure
        , std::string
        , action_modifier::values
        , string_list
    >
{
    member1 name;
    member2 modifiers;
    member3 binds;
};

struct eflags_closure
    : boost::spirit::closure<
          eflags_closure
        , action_modifier::values
    >
{
    member1 values;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_CLOSURES_HPP
