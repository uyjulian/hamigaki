// rotate.hpp: rotate operations

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/integer

#ifndef HAMIGAKI_INTEGER_ROTATE_HPP
#define HAMIGAKI_INTEGER_ROTATE_HPP

#include <boost/assert.hpp>
#include <boost/cstdint.hpp>

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    #include <stdlib.h>
#endif

namespace hamigaki {

inline boost::uint32_t rotate_left(boost::uint32_t n, boost::uint32_t s)
{
    BOOST_ASSERT(s != 0);
    BOOST_ASSERT(s < 32u);

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    return ::_rotl(n, static_cast<int>(s));
#elif defined(__MWERKS__) && (defined(__MC68K__) || defined(__INTEL__))
    return __rol(n, s);
#else
    return (n << s) | (n >> (32-s));
#endif
}

inline boost::uint32_t rotate_right(boost::uint32_t n, boost::uint32_t s)
{
    BOOST_ASSERT(s != 0);
    BOOST_ASSERT(s < 32u);

#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    return ::_rotr(n, static_cast<int>(s));
#elif defined(__MWERKS__) && (defined(__MC68K__) || defined(__INTEL__))
    return __ror(n, s);
#else
    return (n >> s) | (n << (32-s));
#endif
}

} // End namespace hamigaki.

#endif // HAMIGAKI_INTEGER_ROTATE_HPP
