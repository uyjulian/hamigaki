// rule_definition.hpp: bjam rule definition

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_RULE_DEFINITION_HPP
#define HAMIGAKI_BJAM_UTIL_RULE_DEFINITION_HPP

#include <hamigaki/bjam/util/list_of_list.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

namespace hamigaki { namespace bjam {

class context;

struct rule_definition
{
    rule_definition() : exported(true)
    {
    }

    list_of_list parameters;
    boost::shared_ptr<std::string> body;
    boost::function1<string_list,context&> native;
    boost::optional<std::string> module_name;
    bool exported;
};

typedef boost::shared_ptr<rule_definition> rule_def_ptr;

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_RULE_DEFINITION_HPP
