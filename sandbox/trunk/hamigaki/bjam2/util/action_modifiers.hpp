// action_modifiers.hpp: action modifiers

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_ACTION_MODIFIERS_HPP
#define HAMIGAKI_BJAM2_UTIL_ACTION_MODIFIERS_HPP

#if defined(BOOST_SPIRIT_DEBUG)
    #include <ostream>
#endif

namespace hamigaki { namespace bjam2 {

struct action_modifier
{
    enum values
    {
        updated     = 1u << 0,
        together    = 1u << 1,
        ignore      = 1u << 2,
        quietly     = 1u << 3,
        piecemeal   = 1u << 4,
        existing    = 1u << 5,
    };
};

inline action_modifier::values
operator|(action_modifier::values lhs, action_modifier::values rhs)
{
    return static_cast<action_modifier::values>(
        static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)
    );
}

inline action_modifier::values
operator&(action_modifier::values lhs, action_modifier::values rhs)
{
    return static_cast<action_modifier::values>(
        static_cast<unsigned>(lhs) & static_cast<unsigned>(rhs)
    );
}

#if defined(BOOST_SPIRIT_DEBUG)
inline std::ostream& operator<<(std::ostream& os, action_modifier::values x)
{
    const char* delim = "";
    if (x & action_modifier::updated)
    {
        os << delim << "updated";
        delim = " ";
    }
    if (x & action_modifier::together)
    {
        os << delim << "together";
        delim = " ";
    }
    if (x & action_modifier::ignore)
    {
        os << delim << "ignore";
        delim = " ";
    }
    if (x & action_modifier::quietly)
    {
        os << delim << "quietly";
        delim = " ";
    }
    if (x & action_modifier::piecemeal)
    {
        os << delim << "piecemeal";
        delim = " ";
    }
    if (x & action_modifier::existing)
    {
        os << delim << "existing";
        delim = " ";
    }
    return os;
}
#endif

} } // End namespaces bjam2, hamigaki.

#endif // HAMIGAKI_BJAM2_UTIL_ACTION_MODIFIERS_HPP
