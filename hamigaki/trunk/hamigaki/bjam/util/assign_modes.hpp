// assign_modes.hpp: assign modes

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_ASSIGN_MODES_HPP
#define HAMIGAKI_BJAM_UTIL_ASSIGN_MODES_HPP

#if defined(BOOST_SPIRIT_DEBUG)
    #include <ostream>
#endif

namespace hamigaki { namespace bjam {

struct assign_mode
{
    enum values
    {
        set,
        append,
        set_default
    };
};

#if defined(BOOST_SPIRIT_DEBUG)
inline std::ostream& operator<<(std::ostream& os, assign_mode::values x)
{
    if (x == assign_mode::set)
        return os << '=';
    else if (x == assign_mode::append)
        return os << "+=";
    else
        return os << "?=";
}
#endif

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_ASSIGN_MODES_HPP
