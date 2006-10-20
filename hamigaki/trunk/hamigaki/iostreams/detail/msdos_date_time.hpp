//  msdos_date_time.hpp: MS-DOS FTIME structure

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DETAIL_MSDOS_DATE_TIME_HPP
#define HAMIGAKI_IOSTREAMS_DETAIL_MSDOS_DATE_TIME_HPP

#include <hamigaki/binary_io.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>
#include <ctime>

namespace hamigaki { namespace iostreams {

struct msdos_date_time
{
    boost::uint16_t time;
    boost::uint16_t date;

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

} } // End namespaces iostreams, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<iostreams::msdos_date_time>
{
private:
    typedef iostreams::msdos_date_time self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::time, little>,
        member<self, boost::uint16_t, &self::date, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DETAIL_MSDOS_DATE_TIME_HPP
