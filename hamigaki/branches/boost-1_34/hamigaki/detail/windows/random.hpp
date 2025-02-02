//  random.hpp: random number generator

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP
#define HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP

#include <hamigaki/hash.hpp>
#include <boost/cstdint.hpp>
#include <windows.h>

namespace hamigaki { namespace detail { namespace windows {

#if defined(_M_IX86) && defined(__BORLANDC__)
#pragma option push -w-8070
__declspec(naked) inline unsigned long long rdstc()
{
    __emit__(0x0F, 0x31, 0xC3);
}
#pragma option pop
#endif

inline boost::uint32_t random_seed()
{
    std::size_t seed = 0;
#if defined(_M_IX86) && (defined(_MSC_VER) || defined(__MWERKS__))
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
    seed ^= hamigaki::hash_value_ui64(rdstc());
#elif defined(__i386__) && defined(__GNUC__)
    boost::uint32_t low;
    boost::uint32_t high;
    __asm__
    (
        "rdtsc\n\t"
        "movl %%eax, %0\n\t"
        "movl %%edx, %1" ::
        "m"(low), "m"(high) :
        "%eax", "%edx"
    );
    boost::hash_combine(seed, low);
    boost::hash_combine(seed, high);
#else
    boost::hash_combine(seed, ::GetTickCount());
#endif
    boost::hash_combine(seed, ::GetCurrentThreadId());
    return hamigaki::hash_value_to_ui32(seed);
}

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP
