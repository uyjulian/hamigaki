// base_closures.hpp: closures for base_definition

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BASE_CLOSURES_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BASE_CLOSURES_HPP

#include <hamigaki/bjam/util/list_of_list.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/attribute/closure.hpp>
#include <boost/optional.hpp>

namespace hamigaki { namespace bjam {

struct list_closure
    : boost::spirit::closure<
          list_closure
        , string_list
    >
{
    member1 values;
};

struct lol_closure
    : boost::spirit::closure<
          lol_closure
        , list_of_list
    >
{
    member1 values;
};

struct func_closure
    : boost::spirit::closure<
          func_closure
        , string_list
        , string_list
    >
{
    member1 values;
    member2 targets;
};

struct func0_closure
    : boost::spirit::closure<
          func0_closure
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

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BASE_CLOSURES_HPP
