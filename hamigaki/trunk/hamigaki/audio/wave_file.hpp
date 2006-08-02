//  wave_file.hpp: WAVE file device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_WAVE_FILE_HPP
#define HAMIGAKI_AUDIO_WAVE_FILE_HPP

#include <hamigaki/audio/detail/iff_base.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/detail/closer.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/shared_ptr.hpp>

namespace hamigaki { namespace audio {

namespace detail
{

template<typename Source>
class wave_file_source_impl
{
public:
    typedef char char_type;

    struct category :
        boost::iostreams::optimally_buffered_tag,
        boost::iostreams::input_seekable,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        pcm_format_tag {};

    explicit wave_file_source_impl(const Source& src)
        : src_(src), iff_(src_)
    {
        read_format();

        iff_.next_chunk();
        if (!iff_.select_chunk("data"))
            throw BOOST_IOSTREAMS_FAILURE("cannot find fmt chunk");
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
    iff_file_source<Source,little> iff_;
    pcm_format format_;

    void read_format()
    {
        if (!iff_.select_chunk("fmt "))
            throw BOOST_IOSTREAMS_FAILURE("cannot find fmt chunk");

        char buf[16];
        if (boost::iostreams::read(iff_, buf, sizeof(buf)) != sizeof(buf))
            throw BOOST_IOSTREAMS_FAILURE("broken pcm format");

        boost::uint_least16_t tag = decode_uint<little,2>(buf);
        boost::uint_least16_t channels = decode_uint<little,2>(&buf[2]);
        boost::uint_least32_t rate = decode_uint<little,4>(&buf[4]);
        boost::uint_least16_t bits = decode_uint<little,2>(&buf[14]);

        if ((tag != 1) && (tag != 3))
            throw BOOST_IOSTREAMS_FAILURE("unsupoorted pcm format");

        if ((channels == 0) || (rate == 0) || (bits == 0))
            throw BOOST_IOSTREAMS_FAILURE("bad pcm format");

        if (tag == 1)
        {
            if (bits == 8)
                format_.type = uint8;
            else if (bits == 16)
                format_.type = int_le16;
            else
                throw BOOST_IOSTREAMS_FAILURE("unsupported pcm format");
        }
        else
        {
            if (bits == 32)
                format_.type = float_le32;
            else if (bits == 64)
                format_.type = float_le64;
            else
                throw BOOST_IOSTREAMS_FAILURE("unsupported pcm format");
        }

        format_.rate = rate;
        format_.channels = channels;
    }
};

template<typename Sink>
class wave_file_sink_impl
{
public:
    typedef char char_type;

    wave_file_sink_impl(const Sink& sink, const pcm_format& fmt)
        : sink_(sink), format_(fmt), position_(0), is_open_(true)
    {
        write_header();
        write_format();

        write_chunk_header("data", 0);
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

            boost::iostreams::seek(sink_, start_-4, BOOST_IOS::beg);
            encode_uint<little,4>(&buf[0], position_);
            boost::iostreams::write(sink_, buf, sizeof(buf));

            boost::iostreams::seek(sink_, 4, BOOST_IOS::beg);
            encode_uint<little,4>(&buf[0], start_ + 8 + position_ - 8);
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
        std::memcpy(buf, "RIFF", 4);
        std::memset(buf+4, 0, 4);
        std::memcpy(buf+8, "WAVE", 4);

        boost::iostreams::write(sink_, buf, sizeof(buf));
    }

    void write_chunk_header(const char* id, boost::uint_least32_t size)
    {
        char buf[8];
        std::memcpy(buf, id, 4);
        encode_uint<little,4>(&buf[4], size);

        boost::iostreams::write(sink_, buf, sizeof(buf));
    }

    void write_format()
    {
        char buf[16];
        write_chunk_header("fmt ", sizeof(buf));

        unsigned block_sz = format_.block_size();
        if ((format_.type == float_le32) || (format_.type == float_le64))
            encode_uint<little,2>(buf, 3);
        else
            encode_uint<little,2>(buf, 1);
        encode_uint<little,2>(&buf[2], format_.channels);
        encode_uint<little,4>(&buf[4], format_.rate);
        encode_uint<little,4>(&buf[8], format_.rate * block_sz);
        encode_uint<little,2>(&buf[12], block_sz);
        encode_uint<little,2>(&buf[14], format_.bits());

        boost::iostreams::write(sink_, buf, sizeof(buf));
    }
};

} // namespace detail

template<typename Source>
class basic_wave_file_source
{
    typedef detail::wave_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::optimally_buffered_tag,
        boost::iostreams::input_seekable,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        pcm_format_tag {};

    explicit basic_wave_file_source(const Source& src)
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

class wave_file_source
    : public basic_wave_file_source<hamigaki::iostreams::file_source>
{
private:
    typedef basic_wave_file_source<hamigaki::iostreams::file_source> base_type;

public:
    explicit wave_file_source(const std::string& path)
        : base_type(hamigaki::iostreams::file_source(
            path, BOOST_IOS::in|BOOST_IOS::binary))
    {
    }
};

template<typename Source>
inline basic_wave_file_source<Source> make_wave_file_source(const Source& src)
{
    return basic_wave_file_source<Source>(src);
}


template<typename Sink>
class basic_wave_file_sink
{
    typedef detail::wave_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::optimally_buffered_tag,
        boost::iostreams::output,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        pcm_format_tag {};

    basic_wave_file_sink(const Sink& sink, const pcm_format& fmt)
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

class wave_file_sink
    : public basic_wave_file_sink<hamigaki::iostreams::file_sink>
{
private:
    typedef basic_wave_file_sink<hamigaki::iostreams::file_sink> base_type;

public:
    explicit wave_file_sink(const std::string& path, const pcm_format& fmt)
        : base_type(hamigaki::iostreams::file_sink(
            path, BOOST_IOS::out|BOOST_IOS::binary), fmt)
    {
    }
};

template<typename Sink>
inline basic_wave_file_sink<Sink>
make_wave_file_sink(const Sink& sink, const pcm_format& fmt)
{
    return basic_wave_file_sink<Sink>(sink, fmt);
}

} } // End namespaces audio, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::audio::basic_wave_file_source, 1)

#endif // HAMIGAKI_AUDIO_WAVE_FILE_HPP
