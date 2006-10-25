//  lzh_file.hpp: LZH file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_LZH_FILE_HPP

#include <hamigaki/checksum/sum8.hpp>
#include <hamigaki/iostreams/detail/lha/lzh_header.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/filter/lzhuf.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/relative_restrict.hpp>
#include <hamigaki/iostreams/seek.hpp>
#include <hamigaki/iostreams/tiny_restrict.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/compose.hpp>
#include <boost/iostreams/flush.hpp>
#include <boost/assert.hpp>
#include <boost/crc.hpp>
#include <boost/scoped_array.hpp>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>

#if defined(BOOST_HAS_UNISTD_H)
    #define HAMIGAKI_IOSTREAMS_LHA_OS_TYPE 'U'
#else
    #define HAMIGAKI_IOSTREAMS_LHA_OS_TYPE 'M'
#endif

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
    hamigaki::checksum::sum8 cs_;
};


template<class Sink>
class lzh_restriction
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::output
        , boost::iostreams::device_tag
    {};

    lzh_restriction(Sink& sink, bool& overflow,
        boost::iostreams::stream_offset len)
        : sink_(sink), overflow_(overflow)
        , pos_(0), end_(len != -1 ? len : -1)
    {
        BOOST_ASSERT(pos_ != end_);
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        std::streamsize amt = n;
        if ((end_ != -1) && (pos_ + n >= end_))
        {
            overflow_ = true;
            throw give_up_compression();
        }

        std::streamsize result = boost::iostreams::write(sink_, s, amt);
        pos_ += result;
        return result;
    }

private:
    Sink& sink_;
    bool& overflow_;
    boost::iostreams::stream_offset pos_;
    boost::iostreams::stream_offset end_;
};

} // namespace detail

template<class Source>
class basic_lzh_file_source_impl
{
private:
    typedef iostreams::relative_restriction<Source> restricted_type;

    typedef boost::iostreams::composite<
        lzhuf_decompressor,restricted_type> lzhuf_type;

    typedef tiny_restriction<lzhuf_type> restricted_lzhuf_type;

public:
    explicit basic_lzh_file_source_impl(const Source& src)
        : src_(src), next_offset_(0)
    {
    }

    bool next_entry()
    {
        image_.reset();
        boost::iostreams::seek(src_, next_offset_, BOOST_IOS::beg);
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
            hamigaki::checksum::sum8 cs;
            cs.process_bytes(buf+2, sizeof(buf)-2);

            lha::lv0_header lv0;
            hamigaki::binary_read(buf, lv0);

            const std::streamsize rest_size =
                static_cast<std::streamsize>(lv0.header_size - (sizeof(buf)-2));
            if (rest_size <= 0)
                throw std::runtime_error("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            header_.method = lv0.method;
            header_.compressed_size = lv0.compressed_size;
            header_.file_size = lv0.file_size;
            header_.update_time = lv0.update_date_time.to_time_t();
            header_.attributes = lv0.attributes;
            header_.path = read_path(*hsrc, cs);

            skip_unknown_header(*hsrc, cs);
            if (cs.checksum() != lv0.header_checksum)
                throw std::runtime_error("LZH header checksum missmatch");
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
                throw std::runtime_error("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            header_.method = lv1.method;
            header_.compressed_size = lv1.skip_size;
            header_.file_size = lv1.file_size;
            header_.update_time = lv1.update_date_time.to_time_t();

            if (lv1.method == "-lhd-")
                header_.attributes = msdos_attributes::directory;
            else
                header_.attributes = msdos_attributes::archive;

            header_.path = read_path(*hsrc, cs);
            header_.crc16_checksum = read_little16(*hsrc, cs);
            header_.os = get(*hsrc, cs);

            skip_unknown_header(*hsrc, cs);
            next_size = read_little16(src_, cs);
            if (cs.checksum() != lv1.header_checksum)
                throw std::runtime_error("LZH header checksum missmatch");

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
                throw std::runtime_error("bad LZH header size");

            hsrc = restricted_type(src_, 0, rest_size);

            header_.method = lv2.method;
            header_.compressed_size = lv2.compressed_size;
            header_.file_size = lv2.file_size;
            header_.update_time = static_cast<std::time_t>(lv2.update_time);

            if (lv2.method == "-lhd-")
                header_.attributes = msdos_attributes::directory;
            else
                header_.attributes = msdos_attributes::archive;

            header_.crc16_checksum = read_little16(*hsrc, crc);
            header_.os = get(*hsrc, crc);
            next_size = read_little16(*hsrc, crc);
        }
        else
            throw std::runtime_error("unsupported LZH header");

        if ((header_.method != "-lhd-") &&
            (header_.method != "-lh0-") &&
            (header_.method != "-lh4-") &&
            (header_.method != "-lh5-") &&
            (header_.method != "-lh6-") &&
            (header_.method != "-lh7-") )
        {
            throw std::runtime_error("unsupported LZH method");
        }

        read_extended_header(*hsrc, crc, next_size);

        if (lv == '\x01')
            header_.compressed_size -= iostreams::tell_offset(*hsrc);

        next_offset_ = iostreams::tell_offset(src_) + header_.compressed_size;

        if ((header_.attributes & msdos_attributes::directory) == 0)
        {
            restricted_type plain(src_, 0, header_.compressed_size);
            if (header_.method == "-lh0-")
                image_.reset(new detail::lzh_source<restricted_type>(plain));
            else if (header_.method == "-lh4-")
                image_.reset(new_lzhuf_source(plain, 12, header_.file_size));
            else if (header_.method == "-lh5-")
                image_.reset(new_lzhuf_source(plain, 13, header_.file_size));
            else if (header_.method == "-lh6-")
                image_.reset(new_lzhuf_source(plain, 15, header_.file_size));
            else if (header_.method == "-lh7-")
                image_.reset(new_lzhuf_source(plain, 16, header_.file_size));
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
    stream_offset next_offset_;
    boost::crc_16_type crc_;

    static detail::lzh_source<restricted_lzhuf_type>* new_lzhuf_source(
        const restricted_type& plain,
        std::size_t window_bits, boost::int64_t file_size)
    {
        return new detail::lzh_source<restricted_lzhuf_type>(
            tiny_restrict(
                lzhuf_type(lzhuf_decompressor(window_bits), plain),
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
        return hamigaki::decode_uint<little,2>(buf);
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
                throw std::runtime_error("LZH end-mark not found");
            return false;
        }

        if (buffer[0] == '\0')
            return false;

        return true;
    }

    static boost::uint16_t parse_common(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw std::runtime_error("bad LZH common extended header");

        boost::uint16_t header_crc = hamigaki::decode_uint<little,2>(s);

        s[0] = '\0';
        s[1] = '\0';

        return header_crc;
    }

    static boost::filesystem::path
    parse_directory(const char* s, boost::uint32_t n)
    {
        if (s[n-1] != '\xFF')
            throw std::runtime_error("bad LZH directory extended header");

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
        if (n < 2)
            throw std::runtime_error("bad LZH attributes extended header");

        return hamigaki::decode_uint<little,2>(s);
    }

    static lha::windows_timestamp
    parse_windows_timestamp(char* s, boost::uint32_t n)
    {
        if (n < hamigaki::struct_size<lha::windows_timestamp>::type::value)
            throw std::runtime_error("bad LZH timestamp extended header");

        lha::windows_timestamp ts;
        hamigaki::binary_read(s, ts);
        return ts;
    }

    static std::pair<boost::int64_t,boost::int64_t>
    parse_file_size(char* s, boost::uint32_t n)
    {
        if (n < 16)
            throw std::runtime_error("bad LZH file size extended header");

        boost::int64_t comp = hamigaki::decode_int<little,8>(s);
        boost::int64_t org = hamigaki::decode_int<little,8>(s);
        return std::make_pair(comp, org);
    }

    static boost::uint32_t parse_code_page(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw std::runtime_error("bad LZH code page extended header");

        return hamigaki::decode_uint<little,4>(s);
    }

    static boost::uint16_t parse_unix_permission(char* s, boost::uint32_t n)
    {
        if (n < 2)
            throw std::runtime_error("bad LZH permission extended header");

        return hamigaki::decode_uint<little,2>(s);
    }

    static lha::unix_owner
    parse_unix_owner(char* s, boost::uint32_t n)
    {
        if (n < hamigaki::struct_size<lha::unix_owner>::type::value)
            throw std::runtime_error("bad LZH owner extended header");

        lha::unix_owner owner;
        hamigaki::binary_read(s, owner);
        return owner;
    }

    static std::time_t parse_unix_timestamp(char* s, boost::uint32_t n)
    {
        if (n < 4)
            throw std::runtime_error("bad LZH timestamp extended header");

        return static_cast<std::time_t>(hamigaki::decode_int<little,4>(s));
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
                throw std::runtime_error("bad LZH extended header");

            boost::scoped_array<char> buf(new char[next_size]);
            const std::streamsize ssize =
                static_cast<std::streamsize>(next_size);
            if (boost::iostreams::read(nb, buf.get(), ssize) != ssize)
                throw std::runtime_error("bad LZH extended header");

            char* data = buf.get()+1;
            boost::uint16_t size = next_size - 3;
            if (buf[0] == '\0')
                header_crc = parse_common(data, size);
            else if (buf[0] == '\x01')
                leaf.assign(data, size);
            else if (buf[0] == '\x02')
                branch = parse_directory(data, size);
            else if (buf[0] == '\x40')
                header_.attributes = parse_attributes(data, size);
            else if (buf[0] == '\x41')
                header_.timestamp = parse_windows_timestamp(data, size);
            else if (buf[0] == '\x42')
            {
                boost::tie(header_.compressed_size, header_.file_size)
                    = parse_file_size(data, size);
            }
            else if (buf[0] == '\x46')
                header_.code_page = parse_code_page(data, size);
            else if (buf[0] == '\x50')
                header_.permission = parse_unix_permission(data, size);
            else if (buf[0] == '\x51')
                header_.owner = parse_unix_owner(data, size);
            else if (buf[0] == '\x54')
                header_.update_time = parse_unix_timestamp(data, size);

            crc.process_bytes(buf.get(), next_size);
            next_size = hamigaki::decode_uint<little,2>(data+size);
        }

        if (header_crc)
        {
            if (crc.checksum() != header_crc.get())
                throw std::runtime_error("LZH header CRC missmatch");
        }

        if (header_.path.empty())
            header_.path = branch / leaf;
        else if (!branch.empty())
            header_.path = branch / header_.path;
    }
};


template<class Source>
class basic_lzh_file_source
{
private:
    typedef basic_lzh_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag {};

    explicit basic_lzh_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    bool next_entry()
    {
        return pimpl_->next_entry();
    }

    lha::header header() const
    {
        return pimpl_->header();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
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
    typedef boost::reference_wrapper<Sink> ref_type;
    typedef boost::iostreams::composite<
        lzhuf_compressor, detail::lzh_restriction<Sink>
    > lzhuf_type;

public:
    explicit basic_lzh_file_sink_impl(const Sink& sink)
        : sink_(sink), overflow_(false), method_("-lh5-"), pos_(0)
    {
    }

    void default_method(const char* method)
    {
        method_  = method;
    }

    void create_entry(const lha::header& head)
    {
        if (overflow_)
            throw std::runtime_error("need to rewind the current entry");

        if (image_)
            close();

        header_pos_ = iostreams::tell(sink_);

        header_ = head;
        if (header_.is_directory())
            header_.method = "-lhd-";
        else if (header_.file_size < 3)
            header_.method = "-lh0-";
        else if (header_.method.empty())
            header_.method = method_;

        if ((header_.method != "-lhd-") &&
            (header_.method != "-lh0-") &&
            (header_.method != "-lh4-") &&
            (header_.method != "-lh5-") &&
            (header_.method != "-lh6-") &&
            (header_.method != "-lh7-") )
        {
            throw std::runtime_error("unsupported LZH method");
        }

        write_header();

        if (header_.is_directory())
            return;

        start_pos_ = iostreams::tell(sink_);

        if (header_.method == "-lh0-")
            image_.reset(new detail::lzh_sink<ref_type>(boost::ref(sink_)));
        else if (header_.method == "-lh4-")
            image_.reset(new_lzhuf_sink(12, header_.file_size));
        else if (header_.method == "-lh5-")
            image_.reset(new_lzhuf_sink(13, header_.file_size));
        else if (header_.method == "-lh6-")
            image_.reset(new_lzhuf_sink(15, header_.file_size));
        else if (header_.method == "-lh7-")
            image_.reset(new_lzhuf_sink(16, header_.file_size));
    }

    void rewind_entry()
    {
        image_.reset();
        overflow_ = false;

        iostreams::seek(sink_, header_pos_);

        header_.method = "-lh0-";
        write_header();

        crc_.reset();
        pos_ = 0;
        image_.reset(new detail::lzh_sink<ref_type>(boost::ref(sink_)));
    }

    void close()
    {
        if (header_.is_directory())
            return;

        if (image_)
        {
            image_->flush();
            image_.reset();
        }

        header_.crc16_checksum = crc_.checksum();
        crc_.reset();

        std::streampos next = iostreams::tell(sink_);

        header_.compressed_size = to_offset(next) - to_offset(start_pos_);
        header_.file_size = pos_;
        pos_ = 0;

        iostreams::seek(sink_, header_pos_);
        write_header();

        iostreams::seek(sink_, next);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize amt;
        amt = image_->write(s, n);
        crc_.process_bytes(s, amt);
        pos_ += amt;
        return amt;
    }

    void write_end_mark()
    {
        if (image_)
            close();

        iostreams::blocking_put(sink_, '\0');
        boost::iostreams::close(sink_, BOOST_IOS::out);
    }

private:
    Sink sink_;
    bool overflow_;
    lha::header header_;
    boost::shared_ptr<detail::lzh_sink_base> image_;
    lha::compress_method method_;
    std::streampos header_pos_;
    std::streampos start_pos_;
    boost::crc_16_type crc_;
    boost::int64_t pos_;

    static std::string convert_path(const boost::filesystem::path& ph)
    {
        std::ostringstream os;
        std::copy(ph.begin(), ph.end(),
            std::ostream_iterator<std::string>(os, "\xFF"));
        return os.str();
    }

    template<unsigned char Type, class OtherSink>
    static void write_empty_extended_header(OtherSink& sink)
    {
        static const char buf[3] = { '\x03', '\x00', static_cast<char>(Type) };
        sink.write(buf, 3);
    }

    template<unsigned char Type, class T, class OtherSink>
    static void write_extended_header(OtherSink& sink, const T& x)
    {
        static const char buf[3] =
        {
            static_cast<unsigned char>((binary_size<T>::type::value+3)),
            static_cast<unsigned char>((binary_size<T>::type::value+3) >> 8),
            static_cast<char>(Type)
        };
        sink.write(buf, 3);
        iostreams::binary_write<little>(sink, x);
    }

    template<class OtherSink>
    static void write_extended_header(
        OtherSink& sink, unsigned char type, const std::string& s)
    {
        iostreams::write_uint16<little>(sink, s.size()+3);
        iostreams::blocking_put(sink, static_cast<char>(type));
        if (!s.empty())
            sink.write(&s[0], s.size());
    }

    void write_header()
    {
        lha::lv2_header lv2;
        lv2.header_size = 0;
        lv2.method = header_.method;

        if (header_.compressed_size != -1)
        {
            lv2.compressed_size =
                static_cast<boost::uint32_t>(
                    header_.compressed_size & 0xFFFFFFFF);
        }
        else
            lv2.compressed_size = 0;

        if (header_.file_size != -1)
        {
            lv2.file_size =
                static_cast<boost::uint32_t>(
                    header_.file_size & 0xFFFFFFFF);
        }
        else
            lv2.file_size = 0;

        lv2.update_time = header_.update_time;
        lv2.reserved = static_cast<boost::uint8_t>(msdos_attributes::archive);
        lv2.level = 2;

        std::string buffer;
        boost::iostreams::back_insert_device<std::string> tmp(buffer);
        iostreams::binary_write(tmp, lv2);
        if (header_.crc16_checksum)
            iostreams::write_uint16<little>(tmp, header_.crc16_checksum.get());
        else
            tmp.write("\0", 2);

        if (header_.os)
            boost::iostreams::put(tmp, header_.os.get());
        else
            boost::iostreams::put(tmp, HAMIGAKI_IOSTREAMS_LHA_OS_TYPE);

        if (header_.code_page)
            write_extended_header<0x46>(tmp, header_.code_page.get());

        if (header_.is_directory())
        {
            write_empty_extended_header<0x01>(tmp);
            write_extended_header(tmp, 0x02, convert_path(header_.path));
        }
        else
        {
            write_extended_header(tmp, 0x01, header_.path.leaf());

            const boost::filesystem::path& ph = header_.path.branch_path();
            if (!ph.empty())
                write_extended_header(tmp, 0x02, convert_path(ph));
        }

        if (header_.attributes != msdos_attributes::archive)
            write_extended_header<0x40>(tmp, header_.attributes);

        if (header_.timestamp)
            write_extended_header<0x41>(tmp, header_.timestamp.get());

        if ((header_.compressed_size != -1) && (header_.file_size != -1))
        {
            if (((header_.compressed_size >> 32) != 0) ||
                ((header_.file_size >> 32) != 0))
            {
                write_extended_header<0x42>(tmp,
                    lha::file_size_header(
                        header_.compressed_size, header_.file_size
                    )
                );
            }
        }

        if (header_.permission)
            write_extended_header<0x50>(tmp, header_.permission.get());

        if (header_.owner)
            write_extended_header<0x51>(tmp, header_.owner.get());

        tmp.write("\x06\x00\x00", 3);
        std::size_t crc_off = buffer.size();
        // TODO: timezone
        tmp.write("\x00\x00\x07", 3);

        // end of extended headers
        iostreams::write_uint16<little>(tmp, 0);

        hamigaki::encode_uint<little,2>(&buffer[0], buffer.size());

        boost::crc_16_type crc;
        crc.process_bytes(&buffer[0], buffer.size());
        hamigaki::encode_uint<little,2>(&buffer[crc_off], crc.checksum());

        boost::iostreams::write(sink_, &buffer[0], buffer.size());
    }

    detail::lzh_sink<lzhuf_type>* new_lzhuf_sink(
        std::size_t window_bits, boost::int64_t file_size)
    {
        overflow_ = false;
        return new detail::lzh_sink<lzhuf_type>(
            lzhuf_type(
                lzhuf_compressor(window_bits),
                detail::lzh_restriction<Sink>(sink_, overflow_, file_size)
            )
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

    void rewind_entry()
    {
        pimpl_->rewind_entry();
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
