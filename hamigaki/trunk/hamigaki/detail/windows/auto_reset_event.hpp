// auto_reset_event.hpp: auto-reset event object

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_AUTO_RESET_EVENT_HPP
#define HAMIGAKI_DETAIL_WINDOWS_AUTO_RESET_EVENT_HPP

#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <windows.h>

namespace hamigaki { namespace detail { namespace windows {

class auto_reset_event : boost::noncopyable
{
public:
    explicit auto_reset_event(bool init=true)
        : handle_(::CreateEvent(0, FALSE, init ? TRUE : FALSE, 0))
    {
        if (handle_ == 0)
            throw std::runtime_error("cannot create event object");
    }

    ~auto_reset_event()
    {
        ::CloseHandle(handle_);
    }

    ::HANDLE get() const
    {
        return handle_;
    }

    void set()
    {
        ::SetEvent(handle_);
    }

    void wait()
    {
        ::WaitForSingleObject(handle_, INFINITE);
    }

private:
    ::HANDLE handle_;
};

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_AUTO_RESET_EVENT_HPP
