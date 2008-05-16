// random.hpp: random number generator

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_POSIX_RANDOM_HPP
#define HAMIGAKI_DETAIL_POSIX_RANDOM_HPP

#include <boost/config.hpp>

#include <hamigaki/hash.hpp>
#include <boost/cstdint.hpp>
#include <ctime>
#include <time.h>
#include <unistd.h>

#if defined(BOOST_HAS_GETTIMEOFDAY)
    #include <sys/time.h>
#endif

#if !defined(BOOST_DISABLE_THREADS)
    #if defined(BOOST_HAS_PTHREADS)
        #include <pthread.h>
    #else
        #error unsupported platform
    #endif
#endif

namespace hamigaki { namespace detail { namespace posix {

inline boost::uint32_t random_seed()
{
    std::size_t seed = 0;

#if defined(__i386__)
    boost::uint32_t low;
    boost::uint32_t high;
    __asm__("rdtsc" : "=a"(low), "=d"(high));
    boost::hash_combine(seed, low);
    boost::hash_combine(seed, high);
#elif defined(BOOST_HAS_GETTIMEOFDAY)
    ::timeval tv;
    ::gettimeofday(&tv, 0);
    seed ^= hamigaki::hash_value_integer(tv.tv_sec);
    seed ^= hamigaki::hash_value_integer(tv.tv_usec);
#elif defined(BOOST_HAS_CLOCK_GETTIME)
    ::timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts)
    seed ^= hamigaki::hash_value_integer(ts.tv_sec);
    seed ^= hamigaki::hash_value_integer(ts.tv_nsec);
#else
    seed ^= hamigaki::hash_value_integer(std::time(0));
#endif

    seed ^= hamigaki::hash_value_integer(::getpid());

#if !defined(BOOST_DISABLE_THREADS)
    ::pthread_t tid = ::pthread_self();
    boost::hash_range(
        seed,
        reinterpret_cast<unsigned char*>(&tid),
        reinterpret_cast<unsigned char*>(&tid + 1)
    );
#endif
    return hamigaki::hash_value_to_ui32(seed);
}

} } } // End namespaces posix, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_POSIX_RANDOM_HPP
