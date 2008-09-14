// utc_time.hpp: boost::xtime wrapper for UTC

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/thread for library home page.

#ifndef HAMIGAKI_THREAD_UTC_TIME_HPP
#define HAMIGAKI_THREAD_UTC_TIME_HPP

#include <boost/thread/xtime.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION < 103700
    #include <boost/strong_typedef.hpp>
#else
    #include <boost/serialization/strong_typedef.hpp>
#endif

namespace hamigaki { namespace thread {

BOOST_STRONG_TYPEDEF(boost::xtime::xtime_sec_t, seconds)
BOOST_STRONG_TYPEDEF(boost::xtime::xtime_nsec_t, nanoseconds)

inline nanoseconds microseconds(int usec)
{
    return hamigaki::thread::nanoseconds(usec*1000);
}

inline nanoseconds milliseconds(int msec)
{
    return hamigaki::thread::nanoseconds(msec*1000*1000);
}

struct utc_time : boost::xtime
{
    utc_time()
    {
        boost::xtime_get(this, boost::TIME_UTC);
    }

    utc_time& operator+=(const seconds& n)
    {
        sec += n;
        return *this;
    }

    utc_time& operator+=(const nanoseconds& n)
    {
        nsec += n;
        if (nsec >= 1000*1000*1000)
        {
            ++sec;
            nsec -= 1000*1000*1000;
        }
        return *this;
    }
};

} } // End namespaces thread, hamigaki.

#endif // HAMIGAKI_THREAD_UTC_TIME_HPP
