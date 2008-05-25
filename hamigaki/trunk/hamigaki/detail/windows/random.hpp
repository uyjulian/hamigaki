// random.hpp: random number generator

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP
#define HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP

#include <hamigaki/hash.hpp>
#include <boost/cstdint.hpp>
#include <windows.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1300)
    #include <intrin.h>
#endif

namespace hamigaki { namespace detail { namespace windows {

#if defined(_M_IX86) && defined(__BORLANDC__)
#pragma option push -w-8070
__declspec(naked) inline unsigned long long rdtsc()
{
    __emit__(0x0F, 0x31, 0xC3);
}
#pragma option pop
#endif

inline boost::uint32_t random_seed()
{
    std::size_t seed = 0;
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
    seed ^= hamigaki::hash_value_ui64(__rdtsc());
#elif defined(_M_IX86) && (defined(_MSC_VER) || defined(__MWERKS__))
    boost::uint32_t low_val;
    boost::uint32_t high_val;
    __asm
    {
        push eax
        push edx
        rdtsc
        mov low_val, eax
        mov high_val, edx
        pop edx
        pop eax
    }
    boost::hash_combine(seed, low_val);
    boost::hash_combine(seed, high_val);
#elif defined(_M_IX86) && defined(__BORLANDC__)
    seed ^= hamigaki::hash_value_ui64(rdtsc());
#elif defined(__i386__) && defined(__GNUC__)
    boost::uint32_t low;
    boost::uint32_t high;
    __asm__("rdtsc" : "=a"(low), "=d"(high));
    boost::hash_combine(seed, high);
    boost::hash_combine(seed, low);
#else
    boost::hash_combine(seed, ::GetTickCount());
#endif
    boost::hash_combine(seed, ::GetCurrentThreadId());
    return hamigaki::hash_value_to_ui32(seed);
}

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP
