// module.hpp: bjam module

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_MODULE_HPP
#define HAMIGAKI_BJAM2_UTIL_MODULE_HPP

#include <hamigaki/bjam2/util/native_rule.hpp>
#include <hamigaki/bjam2/util/rule_table.hpp>
#include <hamigaki/bjam2/util/variable_table.hpp>
#include <boost/optional.hpp>
#include <set>

namespace hamigaki { namespace bjam2 {

struct module
{
    rule_table rules;
    variable_table variables;
    boost::optional<std::string> class_module;
    std::set<std::string> imported_modules;
    std::map<std::string,native_rule> native_rules;
};

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_MODULE_HPP
