// rounding.hpp: integer-rounding operations

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/integer

#ifndef HAMIGAKI_INTEGER_ROUNDING_HPP
#define HAMIGAKI_INTEGER_ROUNDING_HPP

namespace hamigaki {

template<typename T>
inline T round_to_even(T x)
{
    if (static_cast<T>(x & static_cast<T>(1)) != static_cast<T>(0))
        return ++x;
    else
        return x;
}

template<typename T>
inline T round_to_odd(T x)
{
    if (static_cast<T>(x & static_cast<T>(1)) == static_cast<T>(0))
        return ++x;
    else
        return x;
}

} // End namespace hamigaki.

#endif // HAMIGAKI_INTEGER_ROUNDING_HPP
