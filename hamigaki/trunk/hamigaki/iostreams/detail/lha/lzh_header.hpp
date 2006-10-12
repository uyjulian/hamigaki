//  lzh_header.hpp: LZH header

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DETAIL_LHA_LZH_HEADER_HPP
#define HAMIGAKI_IOSTREAMS_DETAIL_LHA_LZH_HEADER_HPP

#include <hamigaki/binary_io.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <ctime>

namespace hamigaki { namespace iostreams { namespace lha {

struct compress_method
    : boost::equality_comparable<compress_method
    , boost::equality_comparable2<compress_method, const char*
      > >
{
    char id[5];

    compress_method()
    {
        std::memset(id, 0, 5);
    }

    explicit compress_method(const char* s)
    {
        std::memcpy(id, s, 5);
    }

    compress_method& operator=(const char* s)
    {
        std::memcpy(id, s, 5);
        return *this;
    }

    bool operator==(const compress_method& rhs) const
    {
        return std::memcmp(id, rhs.id, 5) == 0;
    }

    bool operator==(const char* rhs) const
    {
        return std::memcmp(id, rhs, 5) == 0;
    }

    bool empty() const
    {
        return id[0] == '\0';
    }
};

struct attributes
{
    static const boost::uint16_t read_only  = 0x0001;
    static const boost::uint16_t hidden     = 0x0002;
    static const boost::uint16_t system     = 0x0004;
    static const boost::uint16_t directory  = 0x0010;
    static const boost::uint16_t archive    = 0x0020;

    static const boost::uint16_t mask       = 0x0037;
};

struct windows_timestamp
{
    boost::uint64_t creation_time;
    boost::uint64_t last_write_time;
    boost::uint64_t last_access_time;
};

struct header
{
    compress_method method;
    boost::int64_t compressed_size;
    boost::int64_t file_size;
    std::time_t update_time;
    boost::uint16_t attributes;
    boost::filesystem::path path;
    boost::optional<boost::uint16_t> crc16_checksum;
    boost::optional<char> os;
    boost::optional<windows_timestamp> timestamp;
    boost::optional<boost::uint32_t> code_page;
    boost::optional<boost::uint16_t> permission;

    header()
        : compressed_size(-1), file_size(-1), update_time(-1)
        , attributes(attributes::archive)
    {
    }

    bool is_directory() const
    {
        return (attributes & attributes::directory) != 0;
    }

    std::string path_string() const
    {
        if (is_directory())
            return path.native_directory_string();
        else
            return path.native_file_string();
    }
};

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

struct lv0_header
{
    boost::uint8_t header_size;
    boost::uint8_t header_checksum;
    compress_method method;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    msdos_date_time update_date_time;
    boost::uint16_t attributes;
};

struct lv1_header
{
    boost::uint8_t header_size;
    boost::uint8_t header_checksum;
    compress_method method;
    boost::uint32_t skip_size;
    boost::uint32_t file_size;
    msdos_date_time update_date_time;
    boost::uint8_t reserved;
    boost::uint8_t level;
};

struct lv2_header
{
    boost::uint16_t header_size;
    compress_method method;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    boost::int32_t update_time;
    boost::uint8_t reserved;
    boost::uint8_t level;
};

struct file_size_header
{
    boost::int64_t compressed_size;
    boost::int64_t file_size;

    file_size_header(boost::int64_t compressed_size, boost::int64_t file_size)
        : compressed_size(compressed_size), file_size(file_size)
    {
    }
};

} } } // End namespaces lha, iostreams, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<iostreams::lha::compress_method>
{
private:
    typedef iostreams::lha::compress_method self;

public:
    typedef boost::mpl::single_view<
        member<self, char[5], &self::id>
    > members;
};

template<>
struct struct_traits<iostreams::lha::msdos_date_time>
{
private:
    typedef iostreams::lha::msdos_date_time self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::time, little>,
        member<self, boost::uint16_t, &self::date, little>
    > members;
};

template<>
struct struct_traits<iostreams::lha::windows_timestamp>
{
private:
    typedef iostreams::lha::windows_timestamp self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint64_t, &self::creation_time, little>,
        member<self, boost::uint64_t, &self::last_write_time, little>,
        member<self, boost::uint64_t, &self::last_access_time, little>
    > members;
};

template<>
struct struct_traits<iostreams::lha::lv0_header>
{
private:
    typedef iostreams::lha::lv0_header self;
    typedef iostreams::lha::msdos_date_time date_time_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::header_size>,
        member<self, boost::uint8_t, &self::header_checksum>,
        member<self, iostreams::lha::compress_method, &self::method>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, date_time_type, &self::update_date_time>,
        member<self, boost::uint16_t, &self::attributes, little>
    > members;
};

template<>
struct struct_traits<iostreams::lha::lv1_header>
{
private:
    typedef iostreams::lha::lv1_header self;
    typedef iostreams::lha::msdos_date_time date_time_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::header_size>,
        member<self, boost::uint8_t, &self::header_checksum>,
        member<self, iostreams::lha::compress_method, &self::method>,
        member<self, boost::uint32_t, &self::skip_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, date_time_type, &self::update_date_time>,
        member<self, boost::uint8_t, &self::reserved>,
        member<self, boost::uint8_t, &self::level>
    > members;
};

template<>
struct struct_traits<iostreams::lha::lv2_header>
{
private:
    typedef iostreams::lha::lv2_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::header_size, little>,
        member<self, iostreams::lha::compress_method, &self::method>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, boost::int32_t, &self::update_time, little>,
        member<self, boost::uint8_t, &self::reserved>,
        member<self, boost::uint8_t, &self::level>
    > members;
};

template<>
struct struct_traits<iostreams::lha::file_size_header>
{
private:
    typedef iostreams::lha::file_size_header self;

public:
    typedef boost::mpl::list<
        member<self, boost::int64_t, &self::compressed_size, little>,
        member<self, boost::int64_t, &self::file_size, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DETAIL_LHA_LZH_HEADER_HPP
