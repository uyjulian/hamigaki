// timestamp.hpp: the time stamp structure

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/sys for library home page.

#ifndef HAMIGAKI_FILESYSTEM_TIMESTAMP_HPP
#define HAMIGAKI_FILESYSTEM_TIMESTAMP_HPP

#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <ctime>

namespace hamigaki { namespace filesystem {

struct timestamp : boost::totally_ordered<timestamp>
{
    boost::int64_t seconds;
    boost::uint32_t nanoseconds;

    timestamp() : seconds(0), nanoseconds(0)
    {
    }

    timestamp(boost::int64_t sec, boost::uint32_t nsec)
        : seconds(sec), nanoseconds(nsec)
    {
    }

    std::time_t to_time_t() const
    {
        // round up
        if (nanoseconds != 0)
            return static_cast<std::time_t>(seconds + 1);
        else
            return static_cast<std::time_t>(seconds);
    }

    boost::uint64_t to_windows_file_time() const
    {
        // round up
        boost::int64_t sec = seconds;
        boost::uint32_t nsec = nanoseconds + 99;
        if (nsec >= 1000000000)
        {
            nsec -= 1000000000;
            ++sec;
        }

        return
            static_cast<boost::uint64_t>(sec + 11644473600LL) * 10000000ull +
            (nsec / 100);
    }

    static timestamp from_time_t(std::time_t t)
    {
        return timestamp(t, 0);
    }

    static timestamp from_windows_file_time(boost::uint64_t ft)
    {
        boost::int64_t sec = static_cast<boost::int64_t>(ft / 10000000ull);

        boost::uint32_t nsec =
            static_cast<boost::uint32_t>(ft % 10000000ull) * 100;

        return timestamp(sec - 11644473600LL, nsec);
    }

    bool operator<(const timestamp& rhs) const
    {
        return
            (seconds < rhs.seconds) ||
            ((seconds == rhs.seconds) && (nanoseconds < rhs.nanoseconds));
    }

    bool operator==(const timestamp& rhs) const
    {
        return (seconds == rhs.seconds) && (nanoseconds == rhs.nanoseconds);
    }
};

} } // End namespaces filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_TIMESTAMP_HPP
