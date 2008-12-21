// date_time.hpp: ISO 9660 date and time structures

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_DATE_TIME_HPP
#define HAMIGAKI_ARCHIVERS_ISO_DATE_TIME_HPP

#include <boost/detail/workaround.hpp>
#include <boost/version.hpp>

#include <hamigaki/binary/struct_traits.hpp>
#include <hamigaki/filesystem/timestamp.hpp>
#include <hamigaki/dec_format.hpp>
#if BOOST_WORKAROUND(BOOST_VERSION, == 103800)
    #include <boost/date_time/date_defs.hpp> // kepp above posix_time_types.hpp
#endif
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <cstring>

namespace hamigaki { namespace archivers { namespace iso {

namespace iso_detail
{

inline void fill_dec2(char (&buf)[2], boost::uint8_t n)
{
    const std::string& s = to_dec<char>(n);

    if (s.size() == 1)
        s.copy(buf+1, 1);
    else
        s.copy(buf, 2);
}

} // namespace iso_detail

struct date_time
{
    char year[4];
    char month[2];
    char day[2];
    char hour[2];
    char minute[2];
    char second[2];
    char centisecond[2];
    boost::int8_t timezone;

    date_time()
    {
        std::memset(year,        '0', 4);
        std::memset(month,       '0', 2);
        std::memset(day,         '0', 2);
        std::memset(hour,        '0', 2);
        std::memset(minute,      '0', 2);
        std::memset(second,      '0', 2);
        std::memset(centisecond, '0', 2);
        timezone = 0;
    }

    bool empty() const
    {
        return
            (std::memcmp(year,        "0000", 4) == 0) &&
            (std::memcmp(month,       "00",   2) == 0) &&
            (std::memcmp(day,         "00",   2) == 0) &&
            (std::memcmp(hour,        "00",   2) == 0) &&
            (std::memcmp(minute,      "00",   2) == 0) &&
            (std::memcmp(second,      "00",   2) == 0) &&
            (std::memcmp(centisecond, "00",   2) == 0) &&
            (timezone                            == 0) ;
    }

    boost::optional<filesystem::timestamp> to_timestamp() const
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        if (empty())
            return boost::optional<filesystem::timestamp>();

        unsigned short y = from_dec<unsigned short>(year, year+4);
        unsigned short m = from_dec<unsigned short>(month, month+2);
        unsigned short d = from_dec<unsigned short>(day, day+2);
        unsigned short hh = from_dec<unsigned short>(hour, hour+2);
        unsigned short mm = from_dec<unsigned short>(minute, minute+2);
        unsigned short ss = from_dec<unsigned short>(second, second+2);
        unsigned short cs =
            from_dec<unsigned short>(centisecond, centisecond+2);

        ptime pt(date(y,m,d), time_duration(hh,mm,ss));
        pt -= boost::posix_time::minutes(timezone*15);

        const ptime origin(date(1970, 1, 1), time_duration());
        const time_duration& du = pt - origin;

        return filesystem::timestamp(du.total_seconds(), cs*10000000ul);
    }

    static date_time from_timestamp(const filesystem::timestamp& ts)
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        date_time tmp;

        ptime pt(date(1970, 1, 1), time_duration());
        pt += hours(static_cast<long>(ts.seconds / 3600));
        pt += seconds(static_cast<long>(ts.seconds % 3600));

        const date& d = pt.date();
        to_dec<char>(static_cast<unsigned>(d.year())).copy(tmp.year, 4);
        iso_detail::fill_dec2(
            tmp.month, static_cast<boost::uint8_t>(d.month()));
        iso_detail::fill_dec2(tmp.day, static_cast<boost::uint8_t>(d.day()));

        const time_duration& td = pt.time_of_day();
        iso_detail::fill_dec2(
            tmp.hour, static_cast<boost::uint8_t>(td.hours()));
        iso_detail::fill_dec2(
            tmp.minute, static_cast<boost::uint8_t>(td.minutes()));
        iso_detail::fill_dec2(
            tmp.second, static_cast<boost::uint8_t>(td.seconds()));

        iso_detail::fill_dec2(tmp.centisecond,
            static_cast<boost::uint8_t>(ts.nanoseconds/10000000ul));

        return tmp;
    }
};

struct binary_date_time
{
    boost::uint8_t year;
    boost::uint8_t month;
    boost::uint8_t day;
    boost::uint8_t hour;
    boost::uint8_t minute;
    boost::uint8_t second;
    boost::int8_t timezone;

    binary_date_time()
        : year(0), month(0), day(0)
        , hour(0), minute(0), second(0), timezone(0)
    {
    }

    bool empty() const
    {
        return
            (year        == 0) &&
            (month       == 0) &&
            (day         == 0) &&
            (hour        == 0) &&
            (minute      == 0) &&
            (second      == 0) &&
            (timezone    == 0) ;
    }

    date_time to_date_time() const
    {
        date_time tmp;
        if (!empty())
        {
            to_dec<char>(1900+year).copy(tmp.year, 4);
            iso_detail::fill_dec2(tmp.month, month);
            iso_detail::fill_dec2(tmp.day, day);
            iso_detail::fill_dec2(tmp.hour, hour);
            iso_detail::fill_dec2(tmp.minute, minute);
            iso_detail::fill_dec2(tmp.second, second);
            tmp.timezone = timezone;
        }
        return tmp;
    }

    static binary_date_time from_date_time(const date_time& dt)
    {
        binary_date_time tmp;
        if (!dt.empty())
        {
            tmp.year = from_dec<boost::uint16_t>(dt.year, dt.year+4) - 1900;
            tmp.month = from_dec<boost::uint8_t>(dt.month, dt.month+2);
            tmp.day = from_dec<boost::uint8_t>(dt.day, dt.day+2);
            tmp.hour = from_dec<boost::uint8_t>(dt.hour, dt.hour+2);
            tmp.minute = from_dec<boost::uint8_t>(dt.minute, dt.minute+2);
            tmp.second = from_dec<boost::uint8_t>(dt.second, dt.second+2);
            tmp.timezone = dt.timezone;
        }
        return tmp;
    }

    boost::optional<filesystem::timestamp> to_timestamp() const
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        if (empty())
            return boost::optional<filesystem::timestamp>();

        ptime pt(date(1900+year,month,day), time_duration(hour,minute,second));
        pt -= minutes(timezone*15);

        const ptime origin(date(1970, 1, 1), time_duration());
        return filesystem::timestamp((pt - origin).total_seconds(), 0);
    }

    static binary_date_time from_timestamp(const filesystem::timestamp& ts)
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;

        binary_date_time tmp;

        ptime pt(date(1970, 1, 1), time_duration());
        pt += hours(static_cast<long>(ts.seconds / 3600));
        pt += seconds(static_cast<long>(ts.seconds % 3600));

        const date& d = pt.date();
        tmp.year = static_cast<boost::uint8_t>(d.year() - 1900);
        tmp.month = static_cast<boost::uint8_t>(d.month());
        tmp.day = static_cast<boost::uint8_t>(d.day());

        const time_duration& td = pt.time_of_day();
        tmp.hour = static_cast<boost::uint8_t>(td.hours());
        tmp.minute = static_cast<boost::uint8_t>(td.minutes());
        tmp.second = static_cast<boost::uint8_t>(td.seconds());

        return tmp;
    }
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::date_time>
{
private:
    typedef archivers::iso::date_time self;

public:
    typedef boost::mpl::list<
        member<self, char[4], &self::year>,
        member<self, char[2], &self::month>,
        member<self, char[2], &self::day>,
        member<self, char[2], &self::hour>,
        member<self, char[2], &self::minute>,
        member<self, char[2], &self::second>,
        member<self, char[2], &self::centisecond>,
        member<self, boost::int8_t, &self::timezone>
    > members;
};

template<>
struct struct_traits<archivers::iso::binary_date_time>
{
private:
    typedef archivers::iso::binary_date_time self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::year>,
        member<self, boost::uint8_t, &self::month>,
        member<self, boost::uint8_t, &self::day>,
        member<self, boost::uint8_t, &self::hour>,
        member<self, boost::uint8_t, &self::minute>,
        member<self, boost::uint8_t, &self::second>,
        member<self, boost::int8_t, &self::timezone>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_DATE_TIME_HPP
