// com_release.hpp: IUnknown::Release() functor

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_COM_RELEASE_HPP
#define HAMIGAKI_DETAIL_WINDOWS_COM_RELEASE_HPP

#include <objbase.h>

namespace hamigaki { namespace detail { namespace windows {

// to avoid BOOST_BIND_ENABLE_STDCALL
struct com_release
{
    unsigned long operator()(::IUnknown* p) const
    {
        return p->Release();
    }
};

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_COM_RELEASE_HPP
