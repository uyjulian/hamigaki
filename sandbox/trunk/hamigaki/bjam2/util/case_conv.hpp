// case_conv.hpp: case-conversion algorithms for bjam

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_CASE_CONV_HPP
#define HAMIGAKI_BJAM2_UTIL_CASE_CONV_HPP

#include <string>

namespace hamigaki { namespace bjam2 {

inline void to_lower(std::string& s)
{
    typedef std::string::iterator iter_t;
    for (iter_t i = s.begin(), end = s.end(); i != end; ++i)
    {
        char c = *i;
        if ((c >= 'A') && (c <= 'Z'))
            *i = static_cast<char>(c + ('a'-'A'));
    }
}

inline void to_upper(std::string& s)
{
    typedef std::string::iterator iter_t;
    for (iter_t i = s.begin(), end = s.end(); i != end; ++i)
    {
        char c = *i;
        if ((c >= 'a') && (c <= 'z'))
            *i = static_cast<char>(c - ('a'-'A'));
    }
}

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_CASE_CONV_HPP
