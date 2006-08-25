//  random.hpp: random number generator

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_POSIX_RANDOM_HPP
#define HAMIGAKI_DETAIL_POSIX_RANDOM_HPP

#include <boost/config.hpp>

#include <boost/crc.hpp>
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

template<class Crc, class T>
inline void crc_process_bytes(Crc& crc, const T& t)
{
    crc.process_bytes(&t, sizeof(T));
}

inline boost::uint32_t random_seed()
{
    boost::crc_32_type crc;

#if defined(BOOST_HAS_GETTIMEOFDAY)
    ::timeval tv;
    ::gettimeofday(&tv, 0);
    crc_process_bytes(crc, tv);
#elif defined(BOOST_HAS_CLOCK_GETTIME)
    ::timespec ts;
    if (::clock_gettime(CLOCK_REALTIME, &ts) == 0)
        crc_process_bytes(crc, ts);
    else
        crc_process_bytes(crc, std::time(0));
#else
    crc_process_bytes(crc, std::time(0));
#endif

    crc_process_bytes(crc, ::getpid());

#if !defined(BOOST_DISABLE_THREADS)
    crc_process_bytes(crc, ::pthread_self());
#endif

    return crc.checksum();
}

} } } // End namespaces posix, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_POSIX_RANDOM_HPP
