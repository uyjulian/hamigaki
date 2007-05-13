// com_library.hpp: helper for COM initialization

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_COM_LIBRARY_HPP
#define HAMIGAKI_DETAIL_WINDOWS_COM_LIBRARY_HPP

#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <objbase.h>

namespace hamigaki { namespace detail { namespace windows {

class com_library : boost::noncopyable
{
public:
    com_library()
    {
        if (FAILED(::CoInitialize(0)))
            throw std::runtime_error("CoInitialize() failed");
    }

    ~com_library()
    {
        ::CoUninitialize();
    }
};

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_COM_LIBRARY_HPP
