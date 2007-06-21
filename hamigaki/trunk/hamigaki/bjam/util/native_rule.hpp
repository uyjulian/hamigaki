// native_rule.hpp: bjam native rule definition

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_NATIVE_RULE_HPP
#define HAMIGAKI_BJAM_UTIL_NATIVE_RULE_HPP

#include <hamigaki/bjam/util/list_of_list.hpp>
#include <boost/function.hpp>

namespace hamigaki { namespace bjam {

class context;

struct native_rule
{
    native_rule() : version(1)
    {
    }

    list_of_list parameters;
    boost::function1<string_list,context&> native;
    int version;
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_NATIVE_RULE_HPP
