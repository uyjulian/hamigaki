// punctuators.hpp: bjam punctuators checker

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_PUNCTUATORS_HPP
#define HAMIGAKI_BJAM_UTIL_PUNCTUATORS_HPP

#include <climits> // required for <boost/spirit/phoenix/operators.hpp>
#include <boost/spirit/phoenix.hpp>
#include <algorithm>
#include <cstring>
#include <string>

namespace hamigaki { namespace bjam {

inline bool is_punctor(const std::string& s)
{
    using namespace ::phoenix;

    // must be sorted
    static const char* table[] =
    {
        "!",
        "!=",
        "&",
        "&&",
        "(",
        ")",
        "+=",
        ":",
        ";",
        "<",
        "<=",
        "=",
        ">",
        ">=",
        "?=",
        "[",
        "]",
        "{",
        "|",
        "||",
        "}"
    };

    return std::binary_search(
        table,
        table + sizeof(table)/sizeof(table[0]),
        s.c_str(),
        bind(&std::strcmp)(arg1,arg2) < 0);
}

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_PUNCTUATORS_HPP
