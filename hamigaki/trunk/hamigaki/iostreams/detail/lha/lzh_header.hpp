//  lzh_header.hpp: LZH header

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DETAIL_LHA_LZH_HEADER_HPP
#define HAMIGAKI_IOSTREAMS_DETAIL_LHA_LZH_HEADER_HPP

#include <hamigaki/iostreams/detail/msdos_attributes.hpp>
#include <hamigaki/iostreams/detail/msdos_date_time.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include <cstring>

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

struct windows_timestamp
{
    boost::uint64_t creation_time;
    boost::uint64_t last_write_time;
    boost::uint64_t last_access_time;
};

struct unix_owner
{
    boost::uint16_t gid;
    boost::uint16_t uid;
};

struct header
{
    compress_method method;
    boost::int64_t compressed_size;
    boost::int64_t file_size;
    std::time_t update_time;
    boost::uint16_t attributes;
    boost::filesystem::path path;
    boost::filesystem::path link_path;
    boost::optional<boost::uint16_t> crc16_checksum;
    boost::optional<char> os;
    boost::optional<windows_timestamp> timestamp;
    boost::optional<boost::uint32_t> code_page;
    boost::optional<boost::uint16_t> permission;
    boost::optional<unix_owner> owner;

    header()
        : compressed_size(-1), file_size(-1), update_time(-1)
        , attributes(msdos_attributes::archive)
    {
    }

    bool is_directory() const
    {
        if (link_path.empty())
            return (attributes & msdos_attributes::directory) != 0;
        else
            return false;
    }

    bool is_symbolic_link() const
    {
        return !link_path.empty();
    }

    std::string path_string() const
    {
        if (is_directory())
            return path.native_directory_string();
        else
            return path.native_file_string();
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
struct struct_traits<iostreams::lha::unix_owner>
{
private:
    typedef iostreams::lha::unix_owner self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::gid, little>,
        member<self, boost::uint16_t, &self::uid, little>
    > members;
};

template<>
struct struct_traits<iostreams::lha::lv0_header>
{
private:
    typedef iostreams::lha::lv0_header self;
    typedef iostreams::msdos_date_time date_time_type;

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
    typedef iostreams::msdos_date_time date_time_type;

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
