// variable_expansion.hpp: bjam variable expansion

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_VARIABLE_EXPANSION_HPP
#define HAMIGAKI_BJAM2_UTIL_VARIABLE_EXPANSION_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <hamigaki/bjam2/util/list_of_list.hpp>
#include <hamigaki/bjam2/util/variable_table.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

HAMIGAKI_BJAM2_DECL
const string_list& get_variable_values(
    string_list& buf, const std::string& name, const variable_table& table);

HAMIGAKI_BJAM2_DECL
const string_list& get_variable_values(
    string_list& buf, const std::string& name,
    const variable_table& table, const list_of_list& args);

HAMIGAKI_BJAM2_DECL
string_list expand_variable(
    const std::string& s,
    const variable_table& table, const list_of_list& args);

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_UTIL_VARIABLE_EXPANSION_HPP
