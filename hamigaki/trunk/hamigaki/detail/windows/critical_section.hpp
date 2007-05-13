// critical_section.hpp: critical section object

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_CRITICAL_SECTION_HPP
#define HAMIGAKI_DETAIL_WINDOWS_CRITICAL_SECTION_HPP

#include <boost/noncopyable.hpp>
#include <windows.h>

namespace hamigaki { namespace detail { namespace windows {

class critical_section : boost::noncopyable
{
public:
    class scoped_lock;
    friend class scoped_lock;

    critical_section()
    {
        ::InitializeCriticalSection(&data_);
    }

    ~critical_section()
    {
        ::DeleteCriticalSection(&data_);
    }

    class scoped_lock : boost::noncopyable
    {
    public:
        explicit scoped_lock(critical_section& cs) : ptr_(&cs)
        {
            ptr_->lock();
        }

        ~scoped_lock()
        {
            ptr_->unlock();
        }

    private:
        critical_section* ptr_;
    };

private:
    ::CRITICAL_SECTION data_;

    void lock()
    {
        ::EnterCriticalSection(&data_);
    }

    void unlock()
    {
        ::LeaveCriticalSection(&data_);
    }
};

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_CRITICAL_SECTION_HPP
