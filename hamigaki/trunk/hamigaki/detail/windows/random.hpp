//  random.hpp: random number generator

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP
#define HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP

#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <windows.h>

namespace hamigaki { namespace detail { namespace windows {

template<class Crc, class T>
inline void crc_process_bytes(Crc& crc, const T& t)
{
    crc.process_bytes(&t, sizeof(T));
}

inline boost::uint32_t random_seed()
{
    boost::crc_32_type crc;

    ::LARGE_INTEGER c;
    if (::QueryPerformanceCounter(&c) == TRUE)
        crc_process_bytes(crc, c);
    else
        crc_process_bytes(crc, ::GetTickCount());

    ::SYSTEMTIME st;
    ::GetSystemTime(&st);
    crc_process_bytes(crc, st);

    crc_process_bytes(crc, ::GetCurrentThreadId());

    return crc.checksum();
}

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP
