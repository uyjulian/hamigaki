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
#include <hamigaki/iostreams/tiny_restrict.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/detail/adapter/non_blocking_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/flush.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/mpl/list.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>
#include <iterator>
#include <numeric>
#include <sstream>

#if defined(BOOST_HAS_UNISTD_H)
    #define HAMIGAKI_IOSTREAMS_LHA_OS_TYPE 'U'
#else
    #define HAMIGAKI_IOSTREAMS_LHA_OS_TYPE 'M'
#endif

namespace hamigaki { namespace iostreams { namespace lha {

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
    char method[5];
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    std::time_t update_time;
    boost::uint16_t attributes;
    boost::filesystem::path path;
    boost::optional<boost::uint16_t> crc16_checksum;
    boost::optional<char> os;
    boost::optional<windows_timestamp> timestamp;
    boost::optional<boost::uint16_t> permission;

    header()
        : compressed_size(0), file_size(0), update_time(-1)
        , attributes(attributes::archive)
    {
        std::memset(method, 0, 5);
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

class lzh_sink_base
{
public:
    virtual ~lzh_sink_base() {}
    virtual std::streamsize write(const char* s, std::streamsize n) = 0;
    virtual bool flush() = 0;
};

template<class Sink>
class lzh_sink : public lzh_sink_base
{
public:
    explicit lzh_sink(const Sink& sink) : sink_(sink)
    {
    }

    std::streamsize write(const char* s, std::streamsize n) // virtual
    {
        return boost::iostreams::write(sink_, s, n);
    }

    bool flush() // virtual
    {
        return boost::iostreams::flush(sink_);
    }

private:
    Sink sink_;
};

class lha_checksum
{
public:
    lha_checksum() : sum_(0)
    {
    }

    void process_byte(unsigned char byte)
    {
        sum_ += byte;
    }

    void process_bytes(void const* buffer, std::size_t byte_count)
    {
        sum_ +=
            std::accumulate(
                static_cast<const unsigned char*>(buffer),
                static_cast<const unsigned char*>(buffer) + byte_count,
                0u
            );
    }

    unsigned char checksum()
    {
        return static_cast<unsigned char>(sum_);
    }

private:
    unsigned sum_;
};

class crc16_and_lha_checksum
{
public:
    explicit crc16_and_lha_checksum(boost::crc_16_type& crc) : crc_(crc)
    {
    }

    void process_byte(unsigned char byte)
    {
        crc_.process_byte(byte);
        cs_.process_byte(byte);
    }

    void process_bytes(void const* buffer, std::size_t byte_count)
    {
        crc_.process_bytes(buffer, byte_count);
        cs_.process_bytes(buffer, byte_count);
    }

    unsigned char checksum()
    {
        return cs_.checksum();
    }

private:
    boost::crc_16_type& crc_;
    lha_checksum cs_;
};

} // namespace detail

template<class Source>
class basic_lzh_file_source
{
private:
    typedef hamigaki::iostreams::relative_restriction<Source> restricted_type;

    typedef boost::iostreams::composite<
        lzhuff_decompressor,restricted_type> lzhuff_type;

    typedef tiny_restriction<lzhuff_type> restricted_lzhuff_type;

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
        header_ = lha::header();

        char buf[hamigaki::struct_size<lha::lv0_header>::type::value];
        if (!this->read_basic_header(src_, buf, sizeof(buf)))
            return false;

        boost::crc_16_type crc;

        boost::optional<restricted_type> hsrc;
        boost::uint16_t next_size = 0;
        char lv = buf[sizeof(buf)-1];
        if (lv == '\0')
        {
            detail::lha_checksum cs;
            cs.process_bytes(buf+2, sizeof(buf)-2);

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
            header_.path = read_path(*hsrc, cs);
            header_.crc16_checksum = boost::none;
            header_.os = boost::none;

            skip_unknown_header(*hsrc, cs);
            if (cs.checksum() != lv0.header_checksum)
                throw BOOST_IOSTREAMS_FAILURE("LZH header checksum missmatch");
        }
        else if (lv == '\x01')
        {
            crc.process_bytes(buf, 2);

            detail::crc16_and_lha_checksum cs(crc);
            cs.process_bytes(buf+2, sizeof(buf)-2);

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

            header_.path = read_path(*hsrc, cs);
            header_.crc16_checksum = read_little16(*hsrc, cs);
            header_.os = get(*hsrc, cs);

            skip_unknown_header(*hsrc, cs);
            next_size = read_little16(src_, cs);
            if (cs.checksum() != lv1.header_checksum)
                throw BOOST_IOSTREAMS_FAILURE("LZH header checksum missmatch");

            hsrc = restricted_type(src_, 0, -1);
        }
        else if (lv == '\x02')
        {
            crc.process_bytes(buf, sizeof(buf));

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
            next_size = read_little16(*hsrc, crc);
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

        read_extended_header(*hsrc, crc, next_size);

        if (lv == '\x1')
        {
            boost::iostreams::stream_offset size =
                boost::iostreams::position_to_offset(
                    boost::iostreams::seek(*hsrc, 0, BOOST_IOS::cur));
            header_.compressed_size -= size;
        }

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

    lha::header header() const
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
    lha::header header_;
    boost::shared_ptr<detail::lzh_source_base> image_;
    boost::iostreams::stream_offset next_offset_;
    boost::crc_16_type crc_;

    static detail::lzh_source<restricted_lzhuff_type>* new_lzhuff_source(
        const restricted_type& plain,
        std::size_t window_bits, boost::uint32_t file_size)
    {
        return new detail::lzh_source<restricted_lzhuff_type>(
            tiny_restrict(
                lzhuff_type(lzhuff_decompressor(window_bits), plain),
                file_size
            )
        );
    }

    template<class OtherSource, class Checksum>
    static char get(OtherSource& src, Checksum& cs)
    {
        char c;
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);
        if (boost::iostreams::read(nb, &c, 1) != 1)
            throw boost::iostreams::detail::bad_read();
        cs.process_byte(c);
        return c;
    }

    template<class OtherSource, class Checksum>
    static boost::uint16_t read_little16(OtherSource& src, Checksum& cs)
    {
        char buf[2];
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);
        if (boost::iostreams::read(nb, buf, 2) != 2)
            throw boost::iostreams::detail::bad_read();
        cs.process_bytes(buf, sizeof(buf));
        return hamigaki::decode_uint<hamigaki::little, 2>(buf);
    }

    template<class OtherSource, class Checksum>
    static boost::filesystem::path read_path(OtherSource& src, Checksum& cs)
    {
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);

        char c;
        if (boost::iostreams::read(nb, &c, 1) != 1)
            throw boost::iostreams::detail::bad_read();
        cs.process_byte(c);

        std::streamsize count = static_cast<unsigned char>(c);
        boost::scoped_array<char> buffer(new char[count]);

        if (boost::iostreams::read(nb, buffer.get(), count) != count)
            throw boost::iostreams::detail::bad_read();
        cs.process_bytes(buffer.get(), count);

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

    template<class OtherSource, class Checksum>
    void skip_unknown_header(OtherSource& src, Checksum& cs)
    {
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);

        char buf[256];
        std::streamsize n = boost::iostreams::read(nb, buf, sizeof(buf));
        if (n != -1)
            cs.process_bytes(buf, n);
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

    static boost::uint16_t parse_attributes(char* s, boost::uint32_t n)
    {
        if (n < 1)
            throw BOOST_IOSTREAMS_FAILURE("bad LZH attributes extended header");

        return static_cast<unsigned char>(*s);
    }

    static lha::windows_timestamp
    parse_windows_timestamp(char* s, boost::uint32_t n)
    {
        if (n < hamigaki::struct_size<lha::windows_timestamp>::type::value)
            throw BOOST_IOSTREAMS_FAILURE("bad LZH timestamp extended header");

        lha::windows_timestamp ts;
        hamigaki::binary_read(s, ts);
        return ts;
    }

    static boost::uint16_t parse_unix_permission(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw BOOST_IOSTREAMS_FAILURE("bad LZH permission extended header");

        return hamigaki::decode_uint<hamigaki::little, 2>(s);
    }

    static std::time_t parse_unix_timestamp(char* s, boost::uint32_t n)
    {
        if (n < 4)
            throw BOOST_IOSTREAMS_FAILURE("bad LZH timestamp extended header");

        return static_cast<std::time_t>(
            hamigaki::decode_int<hamigaki::little, 4>(s));
    }

    template<class OtherSource>
    void read_extended_header(
        OtherSource& src, boost::crc_16_type& crc, boost::uint16_t next_size)
    {
        boost::iostreams::non_blocking_adapter<OtherSource> nb(src);

        std::string leaf;
        boost::filesystem::path branch;
        boost::optional<boost::uint16_t> header_crc;
        while (next_size)
        {
            if (next_size < 3)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH extended header");

            boost::scoped_array<char> buf(new char[next_size]);
            const std::streamsize ssize =
                static_cast<std::streamsize>(next_size);
            if (boost::iostreams::read(nb, buf.get(), ssize) != ssize)
                throw BOOST_IOSTREAMS_FAILURE("bad LZH extended header");

            boost::uint16_t size = next_size - 3;
            if (buf[0] == '\0')
                header_crc = parse_common(buf.get()+1, size);
            else if (buf[0] == '\x01')
                leaf.assign(buf.get()+1, size);
            else if (buf[0] == '\x02')
                branch = parse_directory(buf.get()+1, size);
            else if (buf[0] == '\x40')
                header_.attributes = parse_attributes(buf.get()+1, size);
            else if (buf[0] == '\x41')
                header_.timestamp = parse_windows_timestamp(buf.get()+1, size);
            else if (buf[0] == '\x50')
                header_.permission = parse_unix_permission(buf.get()+1, size);
            else if (buf[0] == '\x54')
                header_.update_time = parse_unix_timestamp(buf.get()+1, size);

            crc.process_bytes(buf.get(), next_size);
            next_size =
                hamigaki::decode_uint<hamigaki::little, 2>(buf.get()+1+size);
        }

        if (header_crc)
        {
            if (crc.checksum() != header_crc.get())
                throw BOOST_IOSTREAMS_FAILURE("LZH header CRC missmatch");
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


template<class Sink>
class basic_lzh_file_sink_impl
{
private:
    typedef boost::iostreams::composite<
        lzhuff_compressor,boost::reference_wrapper<Sink>
    > lzhuff_type;

public:
    explicit basic_lzh_file_sink_impl(const Sink& sink) : sink_(sink)
    {
        std::memcpy(method_, "-lh5-", 5);
    }

    void default_method(const char* method)
    {
        std::memcpy(method_, method, 5);
    }

    void create_entry(const lha::header& head)
    {
        if (image_)
            close();

        header_pos_ = boost::iostreams::position_to_offset(
            boost::iostreams::seek(sink_, 0, BOOST_IOS::cur));

        header_ = head;
        if (header_.is_directory())
            std::memcpy(header_.method, "-lhd-", 5);
        else if (!header_.method[0])
            std::memcpy(header_.method, method_, 5);

        if ((std::memcmp(header_.method, "-lhd-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh0-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh4-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh5-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh6-", 5) != 0) &&
            (std::memcmp(header_.method, "-lh7-", 5) != 0) )
        {
            throw BOOST_IOSTREAMS_FAILURE("unsupported LZH method");
        }

        write_header();

        if (header_.is_directory())
            return;

        start_pos_ = boost::iostreams::position_to_offset(
            boost::iostreams::seek(sink_, 0, BOOST_IOS::cur));

        typedef boost::reference_wrapper<Sink> ref_type;
        ref_type ref(sink_);
        if (std::memcmp(header_.method, "-lh0-", 5) == 0)
            image_.reset(new detail::lzh_sink<ref_type>(ref));
        else if (std::memcmp(header_.method, "-lh4-", 5) == 0)
            image_.reset(new_lzhuff_sink(ref, 12));
        else if (std::memcmp(header_.method, "-lh5-", 5) == 0)
            image_.reset(new_lzhuff_sink(ref, 13));
        else if (std::memcmp(header_.method, "-lh6-", 5) == 0)
            image_.reset(new_lzhuff_sink(ref, 15));
        else if (std::memcmp(header_.method, "-lh7-", 5) == 0)
            image_.reset(new_lzhuff_sink(ref, 16));
    }

    void close()
    {
        if (header_.is_directory())
            return;

        image_->flush();
        image_.reset();

        header_.crc16_checksum = crc_.checksum();
        crc_.reset();

        std::streamsize next = boost::iostreams::position_to_offset(
            boost::iostreams::seek(sink_, 0, BOOST_IOS::cur));

        header_.compressed_size =
            static_cast<boost::uint32_t>(next - start_pos_);

        boost::iostreams::seek(sink_, header_pos_, BOOST_IOS::beg);
        write_header();

        boost::iostreams::seek(sink_, next, BOOST_IOS::beg);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt = image_->write(s, n);
        if (amt != -1)
        {
            crc_.process_bytes(s, amt);
            header_.file_size += amt;
        }
        return amt;
    }

    void write_end_mark()
    {
        if (image_)
            close();

        boost::iostreams::put(sink_, '\0');
    }

private:
    Sink sink_;
    lha::header header_;
    boost::shared_ptr<detail::lzh_sink_base> image_;
    char method_[5];
    std::streamsize header_pos_;
    std::streamsize start_pos_;
    boost::crc_16_type crc_;

    template<class OtherSink>
    static void write_little16(OtherSink& sink, boost::uint16_t n)
    {
        char buf[2];
        hamigaki::encode_uint<hamigaki::little,2>(buf, n);
        sink.write(buf, 2);
    }

    static std::string convert_path(const boost::filesystem::path& ph)
    {
        std::ostringstream os;
        std::copy(ph.begin(), ph.end(),
            std::ostream_iterator<std::string>(os, "\xFF"));
        return os.str();
    }

    template<class OtherSink>
    static void write_extended_header(
        OtherSink& sink, boost::uint16_t type, const std::string& s)
    {
        write_little16(sink, s.size()+3);
        boost::iostreams::put(sink, static_cast<unsigned char>(type));
        if (!s.empty())
            sink.write(&s[0], s.size());
    }

    void write_header()
    {
        lha::lv2_header lv2;
        lv2.header_size = 0;
        std::memcpy(lv2.method, header_.method, 5);
        lv2.compressed_size = header_.compressed_size;
        lv2.file_size = header_.file_size;
        lv2.update_time = header_.update_time;
        lv2.reserved = static_cast<boost::uint8_t>(lha::attributes::archive);
        lv2.level = 2;

        std::string buffer;
        boost::iostreams::back_insert_device<std::string> tmp(buffer);
        hamigaki::iostreams::binary_write(tmp, lv2);
        if (header_.crc16_checksum)
            write_little16(tmp, header_.crc16_checksum.get());
        else
            tmp.write("\0", 2);

        if (header_.os)
            boost::iostreams::put(tmp, header_.os.get());
        else
            boost::iostreams::put(tmp, HAMIGAKI_IOSTREAMS_LHA_OS_TYPE);

        if (header_.is_directory())
        {
            tmp.write("\x03\x00\x01", 3);
            write_extended_header(tmp, 0x02, convert_path(header_.path));
        }
        else
        {
            write_extended_header(tmp, 0x01, header_.path.leaf());
            write_extended_header(
                tmp, 0x02, convert_path(header_.path.branch_path()));
        }

        if (header_.attributes != lha::attributes::archive)
        {
            tmp.write("\x04\x00\x40", 3);
            boost::iostreams::put(
                tmp, static_cast<unsigned char>(header_.attributes));
        }

        tmp.write("\x06\x00\x00", 3);
        std::size_t crc_off = buffer.size();
        // TODO: timezone
        tmp.write("\x00\x00\x07", 3);

        // end of extended headers
        write_little16(tmp, 0);

        hamigaki::encode_uint<hamigaki::little,2>(&buffer[0], buffer.size());

        boost::crc_16_type crc;
        crc.process_bytes(&buffer[0], buffer.size());
        hamigaki::encode_uint<
            hamigaki::little,2>(&buffer[crc_off], crc.checksum());

        boost::iostreams::write(sink_, &buffer[0], buffer.size());
    }

    static detail::lzh_sink<lzhuff_type>* new_lzhuff_sink(
        const boost::reference_wrapper<Sink>& plain, std::size_t window_bits)
    {
        return new detail::lzh_sink<lzhuff_type>(
            lzhuff_type(lzhuff_compressor(window_bits), plain)
        );
    }
};

template<class Sink>
class basic_lzh_file_sink
{
private:
    typedef basic_lzh_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    explicit basic_lzh_file_sink(const Sink& sink)
        : pimpl_(new impl_type(sink))
    {
    }

    void default_method(const char* method)
    {
        pimpl_->default_method(method);
    }

    void create_entry(const lha::header& head)
    {
        pimpl_->create_entry(head);
    }

    void close()
    {
        pimpl_->close();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    void write_end_mark()
    {
        pimpl_->write_end_mark();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class lzh_file_sink : public basic_lzh_file_sink<file_sink>
{
    typedef basic_lzh_file_sink<file_sink> base_type;

public:
    explicit lzh_file_sink(const std::string& filename)
        : base_type(file_sink(filename, BOOST_IOS::binary))
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP
