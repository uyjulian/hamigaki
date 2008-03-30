// aiff_file.hpp: AIFF file device

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_AIFF_FILE_HPP
#define HAMIGAKI_AUDIO_AIFF_FILE_HPP

#include <hamigaki/audio/detail/aiff_comm_data.hpp>
#include <hamigaki/audio/detail/iff_base.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/binary_io.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/shared_ptr.hpp>
#include <utility>

namespace hamigaki { namespace audio {

namespace detail
{

inline boost::uint64_t decode_extended(boost::int16_t exp, boost::uint64_t mant)
{
    if ((exp == 0) && (mant == 0))
        return 0;
    else
        return mant >> (63-(exp-16383));
}

inline std::pair<boost::int16_t,boost::uint64_t>
encode_extended(boost::uint64_t n)
{
    if (n == 0)
        return std::pair<boost::int16_t,boost::uint64_t>(0, 0);

    boost::int16_t exp = 16383+63;
    boost::uint64_t mant = n;
    while ((mant & 0x8000000000000000ull) == 0)
    {
        --exp;
        mant <<= 1;
    }
    return std::make_pair(exp, mant);
}

template<typename Source>
class aiff_file_source_impl
{
    typedef boost::iostreams::stream_offset off_t;

public:
    explicit aiff_file_source_impl(const Source& src)
        : src_(src), iff_(src_)
    {
        read_format();

        iff_.next_chunk();
        if (!iff_.select_chunk("SSND"))
            throw BOOST_IOSTREAMS_FAILURE("cannot find SSND chunk");

        char buf[8];
        iff_.read(buf, sizeof(buf));
        off_t offset = decode_uint<big,4>(&buf[0]);

        if (offset != 0)
            iff_.seek(offset, BOOST_IOS::cur);
        start_ = 8 + offset;
        position_ = 0;
    }

    pcm_format format() const
    {
        return format_;
    }

    boost::iostreams::stream_offset total() const
    {
        return size_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        off_t rest = size_ - position_;
        if (rest < static_cast<off_t>(n))
            n = static_cast<std::streamsize>(rest);
        if (n == 0)
            return -1;

        std::streamsize amt = iff_.read(s, n);
        if (amt != n)
            throw BOOST_IOSTREAMS_FAILURE("read error");
        position_ += amt;
        return amt;
    }

    void close()
    {
        boost::iostreams::close(src_, BOOST_IOS::in);
    }

    std::streampos seek(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        if (way == BOOST_IOS::beg)
        {
            if ((off < 0) || (off > size_))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = start_ + off;
        }
        else if (way == BOOST_IOS::cur)
        {
            // Optimization for "tell"
            if (off == 0)
                return position_;

            off_t pos = position_ + off;
            if ((pos < 0) || (pos > size_))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
        }
        else
        {
            off_t pos = size_ + off;
            if ((pos < 0) || (pos > size_))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = pos - size_;
        }

        std::streampos pos = iff_.seek(off, way);
        position_ = boost::iostreams::position_to_offset(pos) - start_;
        return boost::iostreams::offset_to_position(position_);
    }

private:
    Source src_;
    iff_file_source<Source,big> iff_;
    pcm_format format_;
    off_t size_;
    off_t start_;
    off_t position_;

    void read_format()
    {
        if (!iff_.select_chunk("COMM"))
            throw BOOST_IOSTREAMS_FAILURE("cannot find COMM chunk");

        aiff_comm_data comm;
        iostreams::binary_read(iff_, comm);
        boost::int16_t channels = comm.num_channels;
        boost::uint32_t samples = comm.num_sample_frames;
        boost::int16_t bits = comm.sample_size;
        boost::uint64_t rate =
            decode_extended(comm.sample_rate_exp, comm.sample_rate_mant);

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

        size_ = format_.block_size() * samples;
    }
};

template<typename Sink>
class aiff_file_sink_impl
{
public:
    aiff_file_sink_impl(const Sink& sink, const pcm_format& fmt)
        : sink_(sink), iff_(sink_, "FORM", "AIFF"), format_(fmt)
    {
        write_format();

        iff_.create_chunk("SSND");

        char buf[8];
        std::memset(buf, 0, sizeof(buf));
        iff_.write(buf, sizeof(buf));
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

        try
        {
            boost::uint32_t size = static_cast<boost::uint32_t>(
                boost::iostreams::position_to_offset(
                    boost::iostreams::seek(iff_, 0, BOOST_IOS::cur))
            ) - 8;

            iff_.close();

            boost::iostreams::seek(sink_, frames_offset_, BOOST_IOS::beg);

            char buf[4];
            encode_int<big,4>(&buf[0], size/format_.block_size());
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
    iff_file_sink<Sink,big> iff_;
    pcm_format format_;
    boost::iostreams::stream_offset frames_offset_;

    void write_format()
    {
        iff_.create_chunk("COMM");

        frames_offset_ =
            boost::iostreams::position_to_offset(
                boost::iostreams::seek(sink_, 0, BOOST_IOS::cur)
            ) + 2;

        aiff_comm_data comm;
        comm.num_channels = format_.channels;
        comm.num_sample_frames = 0;
        comm.sample_size = format_.bits();
        boost::tie(comm.sample_rate_exp, comm.sample_rate_mant) =
            encode_extended(format_.rate);

        iostreams::binary_write(iff_, comm);
    }
};

} // namespace detail

template<typename Source>
class basic_aiff_file_source
{
private:
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

class aiff_file_source
{
private:
    typedef basic_aiff_file_source<iostreams::file_source> impl_type;

public:
    typedef impl_type::char_type char_type;
    typedef impl_type::category category;

    explicit aiff_file_source(const std::string& path)
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
inline basic_aiff_file_source<Source> make_aiff_file_source(const Source& src)
{
    return basic_aiff_file_source<Source>(src);
}


template<typename Sink>
class basic_aiff_file_sink
{
private:
    typedef detail::aiff_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::optimally_buffered_tag
        , boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
        , pcm_format_tag
    {};

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
{
private:
    typedef basic_aiff_file_sink<iostreams::file_sink> impl_type;

public:
    typedef impl_type::char_type char_type;
    typedef impl_type::category category;

    aiff_file_sink(const std::string& path, const pcm_format& fmt)
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
inline basic_aiff_file_sink<Sink>
make_aiff_file_sink(const Sink& sink, const pcm_format& fmt)
{
    return basic_aiff_file_sink<Sink>(sink, fmt);
}

} } // End namespaces audio, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::audio::basic_aiff_file_source, 1)

#endif // HAMIGAKI_AUDIO_AIFF_FILE_HPP
