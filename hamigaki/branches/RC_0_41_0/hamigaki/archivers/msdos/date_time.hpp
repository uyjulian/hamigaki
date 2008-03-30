// date_time.hpp: MS-DOS FTIME structure

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_MSDOS_DATE_TIME_HPP
#define HAMIGAKI_ARCHIVERS_MSDOS_DATE_TIME_HPP

#include <boost/config.hpp>

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>
#include <ctime>

#if !defined(BOOST_WINDOWS)
    #include <time.h>
#endif

namespace hamigaki { namespace archivers { namespace msdos {

struct date_time
{
    boost::uint16_t time;
    boost::uint16_t date;

    date_time() : time(0), date(0)
    {
    }

    explicit date_time(std::time_t t)
    {
        if ((t & 1) != 0)
            ++t;

        std::tm lt;

#if defined(BOOST_WINDOWS)
        lt = *std::localtime(&t);
#else
        ::localtime_r(&t, &lt);
#endif

        unsigned year = 1900 + lt.tm_year;
        unsigned month = 1 + lt.tm_mon;
        unsigned day = lt.tm_mday;
        unsigned hour = lt.tm_hour;
        unsigned min = lt.tm_min;
        unsigned sec = lt.tm_sec;

        time = static_cast<boost::uint16_t>(
            (hour << 11) | (min << 5) | (sec >> 1));

        date = static_cast<boost::uint16_t>(
            ((year - 1980) << 9) | (month << 5) | day);
    }

    int year() const
    {
        return 1980 + (date >> 9);
    }

    int month() const
    {
        return (date >> 5) & 0x0F;
    }

    int day() const
    {
        return date & 0x1F;
    }

    int hours() const
    {
        return time >> 11;
    }

    int minutes() const
    {
        return (time >> 5) & 0x3F;
    }

    int seconds() const
    {
        return (time << 1) & 0x3F;
    }

    std::time_t to_time_t() const
    {
        std::tm lt;

        lt.tm_year = year() - 1900;
        lt.tm_mon = month() - 1;
        lt.tm_mday = day();
        lt.tm_hour = hours();
        lt.tm_min = minutes();
        lt.tm_sec = seconds();
        lt.tm_isdst = -1;

        return std::mktime(&lt);
    }
};

} } } // End namespaces msdos, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::msdos::date_time>
{
private:
    typedef archivers::msdos::date_time self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::time, little>,
        member<self, boost::uint16_t, &self::date, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_MSDOS_DATE_TIME_HPP
