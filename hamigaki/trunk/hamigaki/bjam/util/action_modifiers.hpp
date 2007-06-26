// action_modifiers.hpp: action modifiers

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_ACTION_MODIFIERS_HPP
#define HAMIGAKI_BJAM_UTIL_ACTION_MODIFIERS_HPP

#if defined(BOOST_SPIRIT_DEBUG)
    #include <ostream>
#endif

namespace hamigaki { namespace bjam {

struct action_modifier
{
    enum values
    {
        updated,
        together,
        ignore,
        quietly,
        piecemeal,
        existing
    };
};

inline action_modifier::values
operator|(action_modifier::values lhs, action_modifier::values rhs)
{
    return static_cast<action_modifier::values>(
        static_cast<unsigned>(lhs) | static_cast<unsigned>(rhs)
    );
}

#if defined(BOOST_SPIRIT_DEBUG)
inline std::ostream& operator<<(std::ostream& os, action_modifier::values x)
{
    if (x == action_modifier::updated)
        return os << "updated";
    else if (x == action_modifier::together)
        return os << "together";
    else if (x == action_modifier::ignore)
        return os << "ignore";
    else if (x == action_modifier::quietly)
        return os << "quietly";
    else if (x == action_modifier::piecemeal)
        return os << "piecemeal";
    else
        return os << "existing";
}
#endif

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_ACTION_MODIFIERS_HPP
