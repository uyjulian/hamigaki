// to_unsigned.hpp: select the unsigned type with the same size

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/integer

#ifndef HAMIGAKI_INTEGER_DETAIL_TO_UNSIGNED_HPP
#define HAMIGAKI_INTEGER_DETAIL_TO_UNSIGNED_HPP

#include <boost/integer.hpp>

namespace hamigaki { namespace detail {

template<typename T>
struct to_unsigned
{
    typedef typename boost::uint_t<CHAR_BIT*sizeof(T)>::least type;
};

template<>
struct to_unsigned<long long>
{
    typedef unsigned long long type;
};


} } // End namespace detail, hamigaki.

#endif // HAMIGAKI_INTEGER_DETAIL_TO_UNSIGNED_HPP
