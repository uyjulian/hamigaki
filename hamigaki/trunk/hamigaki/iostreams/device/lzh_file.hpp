//  lzh_file.hpp: LZH file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP

#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/filter/lzhuff.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/relative_restrict.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/mpl/list.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>

namespace hamigaki { namespace iostreams { namespace lha {

struct attributes
{
    static const boost::uint16_t read_only  = 0x0001;
    static const boost::uint16_t hidden     = 0x0002;
    static const boost::uint16_t system     = 0x0004;
    static const boost::uint16_t directory  = 0x0010;
    static const boost::uint16_t archive    = 0x0020;
};

struct basic_header
{
    char method[5];
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    std::time_t update_time;
    boost::uint16_t attributes;
    boost::filesystem::path path;
    boost::optional<boost::uint16_t> crc16_checksum;
    boost::optional<char> os;
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
    char method[5];
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    msdos_date_time update_date_time;
    boost::uint16_t attributes;
};

struct lv1_header
{
    boost::uint8_t header_size;
    boost::uint8_t header_checksum;
    char method[5];
    boost::uint32_t skip_size;
    boost::uint32_t file_size;
    msdos_date_time update_date_time;
    boost::uint8_t reserved;
    boost::uint8_t level;
};

struct lv2_header
{
    boost::uint16_t header_size;
    char method[5];
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    boost::int32_t update_time;
    boost::uint8_t reserved;
    boost::uint8_t level;
};

} } } // End namespaces lha, iostreams, hamigaki.

namespace hamigaki {

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
struct struct_traits<iostreams::lha::lv0_header>
{
private:
    typedef iostreams::lha::lv0_header self;
    typedef iostreams::lha::msdos_date_time date_time_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::header_size>,
        member<self, boost::uint8_t, &self::header_checksum>,
        member<self, char[5], &self::method>,
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
        member<self, char[5], &self::method>,
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
        member<self, char[5], &self::method>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, boost::int32_t, &self::update_time, little>,
        member<self, boost::uint8_t, &self::reserved>,
        member<self, boost::uint8_t, &self::level>
    > members;
};

} // End namespace hamigaki.

namespace hamigaki { namespace iostreams {

namespace detail
{

class lzh_source_base
{
public:
    virtual ~lzh_source_base() {}
    virtual std::streamsize read(char* s, std::streamsize n) = 0;
};

template<class Source>
class lzh_source : public lzh_source_base
{
public:
    explicit lzh_source(const Source& src) : src_(src)
    {
    }

    std::streamsize read(char* s, std::streamsize n) // virtual
    {
        return boost::iostreams::read(src_, s, n);
    }

private:
    Source src_;
};

} // namespace detail

template<class Source>
class basic_lzh_file_source
{
private:
    typedef hamigaki::iostreams::relative_restriction<Source> restricted_type;

    typedef boost::iostreams::composite<
        lzhuff_decompressor,restricted_type> lzhuff_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_lzh_file_source(const Source& src)
        : src_(src), next_offset_(0)
    {
        // TODO: skip data before the archives

        if (!next_entry())
            throw BOOST_IOSTREAMS_FAILURE("bad LZH file");
    }

    bool next_entry()
    {
        image_.reset();
        boost::iostreams::seek(src_, next_offset_, std::ios_base::beg);
        crc_.reset();
        header_.path = boost::filesystem::path();

        char buf[hamigaki::struct_size<lha::lv0_header>::type::value];
        if (!this->read_basic_header(src_, buf, sizeof(buf)))
            return false;

        boost::crc_16_type crc;
        crc.process_bytes(buf, sizeof(buf));

        boost::optional<restricted_type> hsrc;
        char lv = buf[sizeof(buf)-1];
        if (lv == '\0')
        {
            lha::lv0_header lv0;
            hamigaki::binary_read(buf, lv0);

            const std::streamsize rest_size =
                static_cast<std::streamsize>(lv0.header_size - (sizeof(buf)-2));
            if (rest_size <= 0)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            std::memcpy(header_.method, lv0.method, 5);
            header_.compressed_size = lv0.compressed_size;
            header_.file_size = lv0.file_size;
            header_.update_time = lv0.update_date_time.to_time_t();
            header_.attributes = lv0.attributes;
            header_.path = read_path(*hsrc, crc);
            header_.crc16_checksum = boost::none;
            header_.os = boost::none;
        }
        else if (lv == '\x01')
        {
            lha::lv1_header lv1;
            hamigaki::binary_read(buf, lv1);

            const std::streamsize rest_size =
                static_cast<std::streamsize>(lv1.header_size - sizeof(buf));
            if (rest_size <= 0)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            std::memcpy(header_.method, lv1.method, 5);
            header_.compressed_size = lv1.skip_size;
            header_.file_size = lv1.file_size;
            header_.update_time = lv1.update_date_time.to_time_t();

            if (std::memcmp(lv1.method, "-lhd-", 5) == 0)
                header_.attributes = lha::attributes::directory;
            else
                header_.attributes = lha::attributes::archive;

            header_.path = read_path(*hsrc, crc);
            header_.crc16_checksum = read_little16(*hsrc, crc);
            header_.os = get(*hsrc, crc);

            boost::iostreams::seek(*hsrc, 0, BOOST_IOS::end);

            hsrc = restricted_type(src_, 0, -1);
        }
        else if (lv == '\x02')
        {
            lha::lv2_header lv2;
            hamigaki::binary_read(buf, lv2);

            const std::streamsize rest_size =
                static_cast<std::streamsize>(lv2.header_size - sizeof(buf));
            if (rest_size <= 0)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            std::memcpy(header_.method, lv2.method, 5);
            header_.compressed_size = lv2.compressed_size;
            header_.file_size = lv2.file_size;
            header_.update_time = static_cast<std::time_t>(lv2.update_time);

            if (std::memcmp(lv2.method, "-lhd-", 5) == 0)
                header_.attributes = lha::attributes::directory;
            else
                header_.attributes = lha::attributes::archive;

            header_.crc16_checksum = read_little16(*hsrc, crc);
            header_.os = get(*hsrc, crc);
        }
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported LZH header");

        if ((std::memcmp(header_.method, "-lhd-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh0-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh4-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh5-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh6-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh7-", 5) != 0) )
        {
            throw BOOST_IOSTREAMS_FAILURE("unsupported LZH method");
        }

        if (lv != '\x0')
            read_extended_header(*hsrc, crc);

        if (lv == '\x1')
        {
            boost::iostreams::stream_offset size =
                boost::iostreams::position_to_offset(
                    boost::iostreams::seek(*hsrc, 0, BOOST_IOS::cur));
            header_.compressed_size -= (size-2);
        }
        else
            boost::iostreams::seek(*hsrc, 0, BOOST_IOS::end);

        next_offset_ =
            boost::iostreams::position_to_offset(
                boost::iostreams::seek(src_, 0, std::ios_base::cur)
            ) + header_.compressed_size;

        if ((header_.attributes & lha::attributes::directory) == 0)
        {
            restricted_type plain(src_, 0, header_.compressed_size);
            if (std::memcmp(header_.method, "-lh0-", 5) == 0)
                image_.reset(new detail::lzh_source<restricted_type>(plain));
            else if (std::memcmp(header_.method, "-lh4-", 5) == 0)
                image_.reset(new_lzhuff_source(plain, 12, header_.file_size));
            else if (std::memcmp(header_.method, "-lh5-", 5) == 0)
                image_.reset(new_lzhuff_source(plain, 13, header_.file_size));
            else if (std::memcmp(header_.method, "-lh6-", 5) == 0)
                image_.reset(new_lzhuff_source(plain, 15, header_.file_size));
            else if (std::memcmp(header_.method, "-lh7-", 5) == 0)
                image_.reset(new_lzhuff_source(plain, 16, header_.file_size));
        }

        return true;
    }

    lha::basic_header header() const
    {
        return header_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize result = image_->read(s, n);
        if (header_.crc16_checksum)
        {
            if (result > 0)
                crc_.process_bytes(s, result);
            if (result == -1)
            {
                if (crc_.checksum() != header_.crc16_checksum.get())
                    throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");
            }
        }
        return result;
    }

private:
    Source src_;
    lha::basic_header header_;
    boost::shared_ptr<detail::lzh_source_base> image_;
    boost::iostreams::stream_offset next_offset_;
    boost::crc_16_type crc_;

    static detail::lzh_source<lzhuff_type>* new_lzhuff_source(
        const restricted_type& plain,
        std::size_t window_bits, boost::uint32_t file_size)
    {
        return new detail::lzh_source<lzhuff_type>(
            lzhuff_type(lzhuff_decompressor(window_bits, file_size), plain)
        );
    }

    template<class Source2>
    static char get(Source2& src, boost::crc_16_type& crc)
    {
        char c;
        boost::iostreams::non_blocking_adapter<Source2> nb(src);
        if (boost::iostreams::read(nb, &c, 1) != 1)
            throw boost::iostreams::detail::bad_read();
        crc.process_byte(c);
        return c;
    }

    template<class Source2>
    static boost::uint16_t read_little16(Source2& src, boost::crc_16_type& crc)
    {
        char buf[2];
        boost::iostreams::non_blocking_adapter<Source2> nb(src);
        if (boost::iostreams::read(nb, buf, 2) != 2)
            throw boost::iostreams::detail::bad_read();
        crc.process_bytes(buf, sizeof(buf));
        return hamigaki::decode_uint<hamigaki::little, 2>(buf);
    }

    template<class Source2>
    static boost::filesystem::path
    read_path(Source2& src, boost::crc_16_type& crc)
    {
        boost::iostreams::non_blocking_adapter<Source2> nb(src);

        char c;
        if (boost::iostreams::read(nb, &c, 1) != 1)
            throw boost::iostreams::detail::bad_read();
        crc.process_byte(c);

        std::streamsize count = static_cast<unsigned char>(c);
        boost::scoped_array<char> buffer(new char[count]);

        if (boost::iostreams::read(nb, buffer.get(), count) != count)
            throw boost::iostreams::detail::bad_read();
        crc.process_bytes(buffer.get(), count);

        const char* start = buffer.get();
        const char* cur = start;
        const char* end = cur + count;
        boost::filesystem::path ph;

        while (cur != end)
        {
            unsigned char uc = static_cast<unsigned char>(*cur);
            if (((uc >  0x80) && (uc < 0xA0)) ||
                ((uc >= 0xE0) && (uc < 0xFD)) )
            {
                if (++cur == end)
                    break;
                ++cur;
            }
            else if (*cur == '\\')
            {
                ph /= std::string(start, cur-start);
                start = ++cur;
            }
            else
                ++cur;
        }
        if (start != cur)
            ph /= std::string(start, cur-start);

        return ph;
    }

    static bool read_basic_header(
        Source& src, char* buffer, std::streamsize size)
    {
        boost::iostreams::non_blocking_adapter<Source> nb(src);

        std::streamsize amt = boost::iostreams::read(nb, buffer, size);

        if (amt < size)
        {
            if ((amt <= 0) || (buffer[0] != '\0'))
                throw BOOST_IOSTREAMS_FAILURE("LZH end-mark not found");
            return false;
        }

        if (buffer[0] == '\0')
            return false;

        return true;
    }

    static boost::uint16_t parse_common(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw BOOST_IOSTREAMS_FAILURE("bad LZH common extended header");

        boost::uint16_t header_crc =
            hamigaki::decode_uint<hamigaki::little,2>(s);

        s[0] = '\0';
        s[1] = '\0';

        return header_crc;
    }

    static boost::filesystem::path
    parse_directory(const char* s, boost::uint32_t n)
    {
        if (s[n-1] != '\xFF')
            throw BOOST_IOSTREAMS_FAILURE("bad LZH directory extended header");

        const char* cur = s;
        const char* end = s + n;
        boost::filesystem::path ph;

        while (cur != end)
        {
            const char* delim =
                static_cast<const char*>(std::memchr(cur, '\xFF', end-cur));

            ph /= std::string(cur, delim-cur);
            cur = ++delim;
        }

        return ph;
    }

    template<class Source2>
    void read_extended_header(Source2& src, boost::crc_16_type& crc)
    {
        boost::iostreams::non_blocking_adapter<Source2> nb(src);

        std::string leaf;
        boost::filesystem::path branch;
        boost::optional<boost::uint16_t> header_crc;
        while (boost::uint16_t size = read_little16(src, crc))
        {
            if (size < 3)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH extended header");
            size -= 2;

            boost::scoped_array<char> buf(new char[size]);
            const std::streamsize ssize = static_cast<std::streamsize>(size);
            if (boost::iostreams::read(nb, buf.get(), ssize) != ssize)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH extended header");

            if (buf[0] == '\0')
                header_crc = parse_common(buf.get()+1, size-1);
            else if (buf[0] == '\1')
                leaf.assign(buf.get()+1, size-1);
            else if (buf[0] == '\2')
                branch = parse_directory(buf.get()+1, size-1);

            crc.process_bytes(buf.get(), size);
        }

        if (header_crc)
        {
            if (crc.checksum() != header_crc.get())
                throw BOOST_IOSTREAMS_FAILURE("CRC missmatch");
        }

        if (header_.path.empty())
            header_.path = branch / leaf;
        else if (!branch.empty())
            header_.path = branch / header_.path;
    }
};

class lzh_file_source : public basic_lzh_file_source<file_source>
{
    typedef basic_lzh_file_source<file_source> base_type;

public:
    explicit lzh_file_source(const std::string& filename)
        : base_type(file_source(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP
