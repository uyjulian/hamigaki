// random.hpp: random number generator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_RANDOM_HPP
#define HAMIGAKI_DETAIL_RANDOM_HPP

#include <boost/random/linear_congruential.hpp>
#include <boost/config.hpp>
#include <limits>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/detail/windows/random.hpp>
#elif defined(BOOST_HAS_UNISTD_H)
    #include <hamigaki/detail/posix/random.hpp>
#else
    #error "Sorry, unsupported architecture"
#endif

namespace hamigaki { namespace detail {

inline boost::uint32_t random_seed()
{
#if defined(BOOST_WINDOWS)
    return windows::random_seed();
#elif defined(BOOST_HAS_UNISTD_H)
    return posix::random_seed();
#endif
}

inline boost::int32_t random_i32(boost::uint32_t seed)
{
#if defined(BOOST_NO_INTEGRAL_INT64_T)
    return boost::minstd_rand(seed)();
#else
    return boost::rand48(static_cast<boost::int32_t>(seed))();
#endif
}

inline boost::int32_t random_i32()
{
    return random_i32(random_seed());
}

inline boost::uint32_t random_ui32(boost::uint32_t seed)
{
    return static_cast<boost::uint32_t>(random_i32(seed));
}

inline boost::uint32_t random_ui32()
{
    return random_ui32(random_seed());
}

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_RANDOM_HPP
