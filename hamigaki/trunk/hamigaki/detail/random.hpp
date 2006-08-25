//  random.hpp: random number generator

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_RANDOM_HPP
#define HAMIGAKI_DETAIL_RANDOM_HPP

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
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
    boost::mt19937 rng(seed);
    boost::uniform_int<boost::int32_t> dist(
        (std::numeric_limits<boost::int32_t>::min)(),
        (std::numeric_limits<boost::int32_t>::max)()
    );

    boost::variate_generator<
        boost::mt19937&,
        boost::uniform_int<boost::int32_t>
    > gen(rng, dist);

    return gen();
}

inline boost::int32_t random_i32()
{
    return random_i32(random_seed());
}

inline boost::uint32_t random_ui32(boost::uint32_t seed)
{
    boost::mt19937 rng(seed);
    boost::uniform_int<boost::uint32_t> dist(0,0xFFFFFFFF);

    boost::variate_generator<
        boost::mt19937&,
        boost::uniform_int<boost::uint32_t>
    > gen(rng, dist);

    return gen();
}

inline boost::uint32_t random_ui32()
{
    return random_ui32(random_seed());
}

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_RANDOM_HPP
