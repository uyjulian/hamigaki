// rule_definition.hpp: bjam rule definition

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_RULE_DEFINITION_HPP
#define HAMIGAKI_BJAM2_UTIL_RULE_DEFINITION_HPP

#include <hamigaki/bjam2/util/action_modifiers.hpp>
#include <hamigaki/bjam2/util/list_of_list.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>

namespace hamigaki { namespace bjam2 {

class context;

struct rule_definition
{
    rule_definition()
        : exported(true)
        , modifiers(static_cast<action_modifier::values>(0))
        , line(1)
    {
    }

    list_of_list parameters;
    boost::function1<string_list,context&> body;
    boost::optional<std::string> module_name;
    bool exported;
    std::string commands;
    action_modifier::values modifiers;
    string_list binds;
    std::string filename;
    int line;
};

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_RULE_DEFINITION_HPP
