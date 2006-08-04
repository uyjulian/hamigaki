//  aiff_file.hpp: AIFF file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_AIFF_FILE_HPP
#define HAMIGAKI_AUDIO_AIFF_FILE_HPP

#include <hamigaki/audio/detail/iff_base.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/shared_ptr.hpp>

namespace hamigaki { namespace audio {

namespace detail
{

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

    struct category :
        boost::iostreams::optimally_buffered_tag,
        boost::iostreams::input_seekable,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        pcm_format_tag {};

    explicit aiff_file_source_impl(const Source& src)
        : src_(src), iff_(src_)
    {
        read_format();

        iff_.next_chunk();
        if (!iff_.select_chunk("SSND"))
            throw BOOST_IOSTREAMS_FAILURE("cannot find SSND chunk");
    }

    std::streamsize optimal_buffer_size() const
    {
        return format_.optimal_buffer_size();
    }

    pcm_format format() const
    {
        return format_;
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return iff_.read(s, n);
    }

    void close()
    {
        boost::iostreams::close(src_, BOOST_IOS::in);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return iff_.seek(off, way);
    }

private:
    Source src_;
    iff_file_source<Source,big> iff_;
    pcm_format format_;

    void read_format()
    {
        if (!iff_.select_chunk("COMM"))
            throw BOOST_IOSTREAMS_FAILURE("cannot find COMM chunk");

        char buf[18];
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
    }
};

template<typename Sink>
class aiff_file_sink_impl
{
public:
    typedef char char_type;

    struct category :
        boost::iostreams::optimally_buffered_tag,
        boost::iostreams::output,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        pcm_format_tag {};

    aiff_file_sink_impl(const Sink& sink, const pcm_format& fmt)
        : sink_(sink), iff_(sink_, "FORM", "AIFF"), format_(fmt)
    {
        iff_.create_chunk("SSND");
    }

    std::streamsize optimal_buffer_size() const
    {
        return format_.optimal_buffer_size();
    }

    pcm_format format() const
    {
        return format_;
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return iff_.write(s, n);
    }

    void close()
    {
        bool nothrow = false;
        boost::iostreams::detail::
            external_closer<Sink> close_sink(sink_, BOOST_IOS::out, nothrow);

        boost::iostreams::detail::external_closer<iff_file_sink<Sink,big> >
            close_iff(iff_, BOOST_IOS::out, nothrow);

        try
        {
            boost::uint_least32_t size = static_cast<boost::uint_least32_t>(
                boost::iostreams::position_to_offset(
                    boost::iostreams::seek(iff_, 0, BOOST_IOS::cur))
            );

            write_format(size/format_.block_size());
        }
        catch (...)
        {
            nothrow = true;
            throw;
        }
    }

private:
    Sink sink_;
    iff_file_sink<Sink,big> iff_;
    pcm_format format_;

    void write_format(boost::uint_least32_t frames)
    {
        iff_.create_chunk("COMM");

        char buf[18];
        encode_int<big,2>(&buf[0], format_.channels);
        encode_int<big,4>(&buf[2], frames);
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
