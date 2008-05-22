// byte_swap.hpp: byte swap operations

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/integer

#ifndef HAMIGAKI_INTEGER_BYTE_SWAP_HPP
#define HAMIGAKI_INTEGER_BYTE_SWAP_HPP

#include <boost/cstdint.hpp>

#if defined(_MSC_VER)
    #include <stdlib.h>
#endif

namespace hamigaki {

inline boost::uint16_t byte_swap16(boost::uint16_t n)
{
#if defined(_MSC_VER)
    return _byteswap_ushort(n);
#elif defined(__MWERKS__) && defined(_M_IX86)
    __asm
    {
        mov ax, n
        rol ax, 8
    }
#elif defined(__GNUC__) && defined(__i386__)
    __asm__("rolw $8, %w0" : "=r"(n) : "0"(n) : "cc");
    return n;
#else
    return (n >> 8) | (n << 8);
#endif
}

inline boost::uint32_t byte_swap32(boost::uint32_t n)
{
#if defined(_MSC_VER)
    return _byteswap_ulong(n);
#elif defined(__MWERKS__) && defined(_M_IX86)
    __asm
    {
        mov eax, n
        bswap eax
    }
#elif defined(__GNUC__) && defined(__i486__)
    __asm__("bswap %0" : "=r"(n) : "0"(n));
    return n;
#elif defined(__GNUC__) && defined(__i386__)
    __asm__
    (
        "rolw $8, %w0\n\t"
        "roll $16, %0\n\t"
        "rolw $8, %w0" :
        "=q"(n) : "0"(n) : "cc"
    );
    return n;
#elif defined(__GNUC__) && defined(__POWERPC__)
    boost::uint32_t result;
    boost::uint32_t* ptr = &result;
    __asm__("stwbrx %1, 0, %2" : "=m"(*ptr) : "r"(n), "r"(ptr));
    return result;
#else
    return (n >> 24) | ((n & 0xFF0000) >> 8) | ((n & 0xFF00) << 8) | (n << 24);
#endif
}

} // End namespace hamigaki.

#endif // HAMIGAKI_INTEGER_BYTE_SWAP_HPP
