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

inline boost::uint32_t random_seed()
{
    std::size_t seed = 0;
    boost::hash_combine(seed, ::GetTickCount());
    boost::hash_combine(seed, ::GetCurrentThreadId());
    return hamigaki::hash_value_to_ui32(seed);
}

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_RANDOM_HPP
