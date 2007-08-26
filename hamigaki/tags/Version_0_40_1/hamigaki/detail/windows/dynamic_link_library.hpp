// dynamic_link_library.hpp: a wrapper class for dynamic link library

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_DYNAMIC_LINK_LIBRARY_HPP
#define HAMIGAKI_DETAIL_WINDOWS_DYNAMIC_LINK_LIBRARY_HPP

#include <boost/noncopyable.hpp>
#include <new>
#include <stdexcept>
#include <string>
#include <windows.h>

namespace hamigaki { namespace detail { namespace windows {

class dynamic_link_library : boost::noncopyable
{
public:
    explicit dynamic_link_library(const char* name)
        : handle_(::LoadLibraryA(name))
    {
        if (handle_ == 0)
        {
            std::string msg("cannot load dll: ");
            msg += name;
            throw std::runtime_error(msg);
        }
    }

    ~dynamic_link_library()
    {
        if (handle_ != 0)
            ::FreeLibrary(handle_);
    }

    ::FARPROC get_proc_address(const char* name, const std::nothrow_t&)
    {
        return ::GetProcAddress(handle_, name);
    }

    ::FARPROC get_proc_address(const char* name)
    {
        ::FARPROC ptr = this->get_proc_address(name, std::nothrow);
        if (ptr == 0)
        {
            std::string msg("cannot find symbol: ");
            msg += name;
            throw std::runtime_error(msg);
        }
        return ptr;
    }

private:
    ::HMODULE handle_;
};

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_DYNAMIC_LINK_LIBRARY_HPP
