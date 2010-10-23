// module.hpp: bjam module

// Copyright Takeshi Mouri 2007-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_MODULE_HPP
#define HAMIGAKI_BJAM_UTIL_MODULE_HPP

#include <hamigaki/bjam/util/native_rule.hpp>
#include <hamigaki/bjam/util/rule_table.hpp>
#include <hamigaki/bjam/util/variable_table.hpp>
#include <boost/optional.hpp>
#include <set>

namespace hamigaki { namespace bjam {

struct module
{
    rule_table rules;
    variable_table variables;
    boost::optional<std::string> class_module;
    std::set<std::string> imported_modules;
    std::map<std::string,native_rule> native_rules;
    bool user_module;

    module() : user_module(false)
    {
    }
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_MODULE_HPP
