//  aiff_file.hpp: AIFF file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_AUDIO_AIFF_FILE_HPP
#define HAMIGAKI_AUDIO_AIFF_FILE_HPP

#include <hamigaki/audio/detail/endian.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/detail/closer.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>

namespace hamigaki { namespace audio {

namespace detail
{

struct iff_chunk_header
{
    char id[4];
    boost::uint_least32_t size;
};

inline boost::uint_least64_t decode_extended(const char* s)
{
    boost::int_least16_t exp = decode_int<big,2>(s);
    boost::uint_least64_t mant = decode_uint<big,8>(s+2);

    if ((exp == 0) && (mant == 0))
        return 0;
    else
        return mant >> (63-(exp-16383));
}

inline void encode_extended(char* s, boost::uint_least64_t n)
{
    if (n == 0)
    {
        std::memset(s, 0, 10);
        return;
    }

    boost::int_least16_t exp = 16383+63;
    boost::uint_least64_t mant = n;
    while ((mant & 0x8000000000000000ull) == 0)
    {
        --exp;
        mant <<= 1;
    }
    encode_int<big,2>(s, exp);
    encode_uint<big,8>(s+2, mant);
}

template<typename Source>
class aiff_file_source_impl
{
public:
    typedef char char_type;

    explicit aiff_file_source_impl(const Source& src)
        : src_(src), position_(0), is_open_(true)
    {
        read_header();
        read_format();

        const iff_chunk_header& chunk = find_chunk("SSND");
        start_ = boost::iostreams::position_to_offset(
            boost::iostreams::seek(src_, 8, BOOST_IOS::cur));
        data_size_ = chunk.size - 8;
    }

    pcm_format format() const
    {
        return format_;
    }

    boost::iostreams::stream_offset total() const
    {
        return data_size_;
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        n = (std::min)(n, data_size_-position_);
        if (n == 0)
            return -1;

        std::streamsize amt = boost::iostreams::read(src_, s, n);
        if (amt != n)
            throw BOOST_IOSTREAMS_FAILURE("PCM read error");
        position_ += amt;
        return amt;
    }

    void close()
    {
        if (is_open_)
        {
            boost::iostreams::close(src_, BOOST_IOS::in);
            position_ = data_size_;
            is_open_ = false;
        }
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        if (way == BOOST_IOS::beg)
        {
            if ((off < 0) || (off > data_size_))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = start_ + off;
            return boost::iostreams::seek(src_, off, way) - start_;
        }
        else if (way == BOOST_IOS::cur)
        {
            std::streamsize pos = position_ + off;
            if ((pos < 0) || (pos > data_size_))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            return boost::iostreams::seek(src_, off, way) - start_;
        }
        else
        {
            std::streamsize pos = data_size_ + off;
            if ((pos < 0) || (pos > data_size_))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = pos - data_size_;
            return boost::iostreams::seek(src_, off, way) - start_;
        }
    }

private:
    Source src_;
    pcm_format format_;
    boost::iostreams::stream_offset start_;
    std::streamsize position_;
    std::streamsize data_size_;
    bool is_open_;

    boost::uint_least32_t read_header()
    {
        char buf[12];
        if (boost::iostreams::read(src_, buf, sizeof(buf)) != sizeof(buf))
            throw BOOST_IOSTREAMS_FAILURE("broken IFF header");

        if (std::memcmp(buf, "FORM", 4) != 0)
            throw BOOST_IOSTREAMS_FAILURE("bad IFF header");

        if (std::memcmp(buf+8, "AIFF", 4) != 0)
            throw BOOST_IOSTREAMS_FAILURE("bad IFF format type");

        return decode_uint<big,4>(buf+4);
    }

    iff_chunk_header raed_chunk_header()
    {
        char buf[8];
        if (boost::iostreams::read(src_, buf, sizeof(buf)) != sizeof(buf))
            throw BOOST_IOSTREAMS_FAILURE("cannot found chunk");

        iff_chunk_header chunk;
        std::memcpy(chunk.id, buf, 4);
        chunk.size = decode_uint<big,4>(buf+4);
        return chunk;
    }

    iff_chunk_header find_chunk(const char* id)
    {
        iff_chunk_header chunk;
        while (true)
        {
            chunk = raed_chunk_header();
            if (std::memcmp(chunk.id, id, 4) == 0)
                break;

            boost::iostreams::seek(src_, chunk.size, BOOST_IOS::cur);
        }
        return chunk;
    }

    void read_format()
    {
        const iff_chunk_header& chunk = raed_chunk_header();

        char buf[18];
        if (chunk.size < sizeof(buf))
            throw BOOST_IOSTREAMS_FAILURE("bad pcm format");

        if (boost::iostreams::read(src_, buf, sizeof(buf)) != sizeof(buf))
            throw BOOST_IOSTREAMS_FAILURE("broken pcm format");

        boost::int_least16_t channels = decode_int<big,2>(&buf[0]);
//      boost::uint_least32_t samples = decode_uint<big,4>(&buf[2]);
        boost::int_least16_t bits = decode_int<big,2>(&buf[6]);
        boost::uint_least64_t rate = decode_extended(&buf[8]);

        if ((channels == 0) || (rate == 0) || (bits == 0))
            throw BOOST_IOSTREAMS_FAILURE("bad pcm format");

        if (bits == 8)
            format_.type = int8;
        else if (bits == 16)
            format_.type = int_be16;
        else if (bits == 24)
            format_.type = int_be24;
        else if (bits == 32)
            format_.type = int_be32;
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported pcm format");

        format_.rate = static_cast<long>(rate);
        format_.channels = channels;

        boost::iostreams::stream_offset rest = chunk.size - sizeof(buf);
        if (rest != 0)
            boost::iostreams::seek(src_, rest, BOOST_IOS::cur);
    }
};

template<typename Sink>
class aiff_file_sink_impl
{
public:
    typedef char char_type;

    aiff_file_sink_impl(const Sink& sink, const pcm_format& fmt)
        : sink_(sink), format_(fmt), position_(0), is_open_(true)
    {
        write_header();
        write_format();

        write_chunk_header("SSND", 0);
        char buf[8];
        std::memset(buf, 0, sizeof(buf));
        boost::iostreams::write(sink_, buf, sizeof(buf));

        start_ = boost::iostreams::position_to_offset(
            boost::iostreams::seek(sink_, 0, BOOST_IOS::cur));
    }

    pcm_format format() const
    {
        return format_;
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        std::streamsize amt = boost::iostreams::write(sink_, s, n);
        if (amt != n)
            throw BOOST_IOSTREAMS_FAILURE("PCM write error");
        position_ += amt;
        return amt;
    }

    void close()
    {
        if (!is_open_)
            return;

        is_open_ = false;

        bool nothrow = false;
        boost::iostreams::detail::
            external_closer<Sink> close_sink(sink_, BOOST_IOS::out, nothrow);

        try
        {
            char buf[4];

            boost::iostreams::seek(sink_, start_-12, BOOST_IOS::beg);
            encode_uint<big,4>(&buf[0], 8 + position_);
            boost::iostreams::write(sink_, buf, sizeof(buf));

            boost::iostreams::seek(sink_, 4, BOOST_IOS::beg);
            encode_uint<big,4>(&buf[0], start_ + position_ - 8);
            boost::iostreams::write(sink_, buf, sizeof(buf));

            boost::iostreams::seek(sink_, 22, BOOST_IOS::beg);
            encode_uint<big,4>(&buf[0], position_/format_.block_size());
            boost::iostreams::write(sink_, buf, sizeof(buf));
        }
        catch (...)
        {
            nothrow = true;
            throw;
        }
    }

private:
    Sink sink_;
    pcm_format format_;
    boost::iostreams::stream_offset start_;
    std::streamsize position_;
    bool is_open_;

    void write_header()
    {
        char buf[12];
        std::memcpy(buf, "FORM", 4);
        std::memset(buf+4, 0, 4);
        std::memcpy(buf+8, "AIFF", 4);

        boost::iostreams::write(sink_, buf, sizeof(buf));
    }

    void write_chunk_header(const char* id, boost::uint_least32_t size)
    {
        char buf[8];
        std::memcpy(buf, id, 4);
        encode_uint<big,4>(&buf[4], size);

        boost::iostreams::write(sink_, buf, sizeof(buf));
    }

    void write_format()
    {
        char buf[18];
        write_chunk_header("COMM", sizeof(buf));

        encode_int<big,2>(&buf[0], format_.channels);
        std::memset(&buf[2], 0, 4);
        encode_int<big,2>(&buf[6], format_.bits());
        encode_extended(&buf[8], format_.rate);

        boost::iostreams::write(sink_, buf, sizeof(buf));
    }
};

} // namespace detail

template<typename Source>
class basic_aiff_file_source
{
    typedef detail::aiff_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::optimally_buffered_tag,
        boost::iostreams::input_seekable,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        pcm_format_tag {};

    explicit basic_aiff_file_source(const Source& src)
        : pimpl_(new impl_type(src))
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        return pimpl_->format().optimal_buffer_size();
    }

    pcm_format format() const
    {
        return pimpl_->format();
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

    void close()
    {
        pimpl_->close();
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return pimpl_->seek(off, way);
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class aiff_file_source
    : public basic_aiff_file_source<hamigaki::iostreams::file_source>
{
private:
    typedef basic_aiff_file_source<hamigaki::iostreams::file_source> base_type;

public:
    explicit aiff_file_source(const std::string& path)
        : base_type(hamigaki::iostreams::file_source(
            path, BOOST_IOS::in|BOOST_IOS::binary))
    {
    }
};

template<typename Source>
inline basic_aiff_file_source<Source> make_aiff_file_source(const Source& src)
{
    return basic_aiff_file_source<Source>(src);
}


template<typename Sink>
class basic_aiff_file_sink
{
    typedef detail::aiff_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::optimally_buffered_tag,
        boost::iostreams::output,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        pcm_format_tag {};

    basic_aiff_file_sink(const Sink& sink, const pcm_format& fmt)
        : pimpl_(new impl_type(sink, fmt))
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        return pimpl_->format().optimal_buffer_size();
    }

    pcm_format format() const
    {
        return pimpl_->format();
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    void close()
    {
        pimpl_->close();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;
};

class aiff_file_sink
    : public basic_aiff_file_sink<hamigaki::iostreams::file_sink>
{
private:
    typedef basic_aiff_file_sink<hamigaki::iostreams::file_sink> base_type;

public:
    explicit aiff_file_sink(const std::string& path, const pcm_format& fmt)
        : base_type(hamigaki::iostreams::file_sink(
            path, BOOST_IOS::out|BOOST_IOS::binary), fmt)
    {
    }
};

template<typename Sink>
inline basic_aiff_file_sink<Sink>
make_aiff_file_sink(const Sink& sink, const pcm_format& fmt)
{
    return basic_aiff_file_sink<Sink>(sink, fmt);
}

} } // End namespaces audio, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::audio::basic_aiff_file_source, 1)

#endif // HAMIGAKI_AUDIO_AIFF_FILE_HPP
