// func_ptr_cast.hpp: func_ptr_cast for i386

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_I386_FUNC_PTR_CAST_HPP
#define HAMIGAKI_DETAIL_I386_FUNC_PTR_CAST_HPP

#include <boost/cstdint.hpp>

namespace hamigaki { namespace detail {

typedef boost::int32_t intptr_t;
typedef boost::uint32_t uintptr_t;

template <class T, class U>
inline T func_ptr_cast(U func_ptr)
{
    return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(func_ptr));
}

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_I386_FUNC_PTR_CAST_HPP
