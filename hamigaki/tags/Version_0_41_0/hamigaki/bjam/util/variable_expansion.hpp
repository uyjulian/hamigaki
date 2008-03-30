// variable_expansion.hpp: bjam variable expansion

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_VARIABLE_EXPANSION_HPP
#define HAMIGAKI_BJAM_UTIL_VARIABLE_EXPANSION_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/util/list_of_list.hpp>
#include <hamigaki/bjam/util/variable_table.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

HAMIGAKI_BJAM_DECL
const string_list& get_variable_values(
    string_list& buf, const std::string& name, const variable_table& table);

HAMIGAKI_BJAM_DECL
const string_list& get_variable_values(
    string_list& buf, const std::string& name,
    const variable_table& table, const list_of_list& args);

HAMIGAKI_BJAM_DECL
string_list expand_variable(
    const std::string& s,
    const variable_table& table, const list_of_list& args);

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_UTIL_VARIABLE_EXPANSION_HPP
