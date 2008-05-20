// wave_file.hpp: WAVE file device

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_WAVE_FILE_HPP
#define HAMIGAKI_AUDIO_WAVE_FILE_HPP

#include <hamigaki/audio/detail/iff_base.hpp>
#include <hamigaki/audio/detail/pcm_wave_format.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/shared_ptr.hpp>

namespace hamigaki { namespace audio {

namespace detail
{

template<typename Source>
class wave_file_source_impl
{
public:
    explicit wave_file_source_impl(const Source& src)
        : src_(src), iff_(src_)
    {
        read_format();

        iff_.next_chunk();
        if (!iff_.select_chunk("data"))
            throw BOOST_IOSTREAMS_FAILURE("cannot find fmt chunk");
    }

    pcm_format format() const
    {
        return format_;
    }

    boost::iostreams::stream_offset total() const
    {
        return iff_.total();
    }

    std::streamsize read(char* s, std::streamsize n)
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

        pcm_wave_format fmt;
        iostreams::binary_read(iff_, fmt);

        boost::uint16_t tag = fmt.format_tag;
        boost::uint16_t channels = fmt.channels;
        boost::uint32_t rate = fmt.samples_per_sec;
        boost::uint16_t bits = fmt.bits_per_sample;

        if ((tag != 1) && (tag != 3) && (tag != 6) && (tag != 7))
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
        else if (tag == 3)
        {
            if (bits == 32)
                format_.type = float_le32;
            else if (bits == 64)
                format_.type = float_le64;
            else
                throw BOOST_IOSTREAMS_FAILURE("unsupported pcm format");
        }
        else if (tag == 6)
        {
            if (bits == 8)
                format_.type = a_law;
            else
                throw BOOST_IOSTREAMS_FAILURE("unsupported pcm format");
        }
        else
        {
            if (bits == 8)
                format_.type = mu_law;
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
    wave_file_sink_impl(const Sink& sink, const pcm_format& fmt)
        : sink_(sink), iff_(sink_, "RIFF", "WAVE"), format_(fmt)
    {
        write_format();

        iff_.create_chunk("data");
    }

    pcm_format format() const
    {
        return format_;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return iff_.write(s, n);
    }

    void close()
    {
        bool nothrow = false;
        detail::device_closer<Sink> close_sink(sink_, nothrow);

        detail::device_closer<
            iff_file_sink<Sink,little>
        > close_iff(iff_, nothrow);
    }

private:
    Sink sink_;
    iff_file_sink<Sink,little> iff_;
    pcm_format format_;

    void write_format()
    {
        iff_.create_chunk("fmt ");

        pcm_wave_format fmt;
        unsigned block_sz = format_.block_size();
        if ((format_.type == float_le32) || (format_.type == float_le64))
            fmt.format_tag = 3;
        else if (format_.type == a_law)
            fmt.format_tag = 6;
        else if (format_.type == mu_law)
            fmt.format_tag = 7;
        else
            fmt.format_tag = 1;
        fmt.channels = format_.channels;
        fmt.samples_per_sec = format_.rate;
        fmt.avg_bytes_per_sec = format_.rate * block_sz;
        fmt.block_align = block_sz;
        fmt.bits_per_sample = format_.bits();

        iostreams::binary_write(iff_, fmt);
    }
};

} // namespace detail

template<typename Source>
class basic_wave_file_source
{
private:
    typedef detail::wave_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::optimally_buffered_tag
        , boost::iostreams::input_seekable
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
        , pcm_format_tag
    {};

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

    boost::iostreams::stream_offset total() const
    {
        return pimpl_->total();
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
{
private:
    typedef basic_wave_file_source<iostreams::file_source> impl_type;

public:
    typedef impl_type::char_type char_type;
    typedef impl_type::category category;

    explicit wave_file_source(const std::string& path)
        : impl_(iostreams::file_source(path, BOOST_IOS::in|BOOST_IOS::binary))
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        return impl_.optimal_buffer_size();
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    boost::iostreams::stream_offset total() const
    {
        return impl_.total();
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

    void close()
    {
        impl_.close();
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return impl_.seek(off, way);
    }

private:
    impl_type impl_;
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

    struct category
        : boost::iostreams::optimally_buffered_tag
        , boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
        , pcm_format_tag
    {};

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
{
private:
    typedef basic_wave_file_sink<iostreams::file_sink> impl_type;

public:
    typedef impl_type::char_type char_type;
    typedef impl_type::category category;

    wave_file_sink(const std::string& path, const pcm_format& fmt)
        : impl_(iostreams::file_sink(
            path, BOOST_IOS::out|BOOST_IOS::binary), fmt)
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        return impl_.optimal_buffer_size();
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return impl_.write(s, n);
    }

    void close()
    {
        impl_.close();
    }

private:
    impl_type impl_;
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
