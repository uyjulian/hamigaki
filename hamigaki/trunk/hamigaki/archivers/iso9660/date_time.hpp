//  date_time.hpp: ISO 9660 date and time structures

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO9660_DATE_TIME_HPP
#define HAMIGAKI_ARCHIVERS_ISO9660_DATE_TIME_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <hamigaki/dec_format.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso9660 {

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
        to_dec<char>(1900+year).copy(tmp.year, 4);
        fill_dec2(tmp.month, month);
        fill_dec2(tmp.day, day);
        fill_dec2(tmp.hour, hour);
        fill_dec2(tmp.minute, minute);
        fill_dec2(tmp.second, second);
        tmp.timezone = timezone;
        return tmp;
    }

private:
    static void fill_dec2(char (&buf)[2], boost::uint8_t n)
    {
        const std::string& s = to_dec<char>(n);

        if (s.size() == 1)
            s.copy(buf+1, 1);
        else
            s.copy(buf, 2);
    }
};

} } } // End namespaces iso9660, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso9660::date_time>
{
private:
    typedef archivers::iso9660::date_time self;

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
struct struct_traits<archivers::iso9660::binary_date_time>
{
private:
    typedef archivers::iso9660::binary_date_time self;

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

#endif // HAMIGAKI_ARCHIVERS_ISO9660_DATE_TIME_HPP
