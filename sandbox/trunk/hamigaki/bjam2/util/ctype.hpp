// ctype.hpp: tiny <cctype>

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_CTYPE_HPP
#define HAMIGAKI_BJAM2_UTIL_CTYPE_HPP

namespace hamigaki { namespace bjam2 {

inline bool is_digit(char c)
{
    return (c >= '0') && (c <= '9');
}

inline bool is_space(char c)
{
    switch (c)
    {
        case ' ':
        case '\t':
        case '\v':
        case '\r':
        case '\n':
        case '\f':
            return true;
        default:
            return false;
    }
}

// Note: bjam assumes ASCII or compatibles
inline bool is_lower(char c)
{
    return (c >= 'a') && (c <= 'z');
}

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_CTYPE_HPP
