// au_file.hpp: Au file device

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_AU_FILE_HPP
#define HAMIGAKI_AUDIO_AU_FILE_HPP

#include <hamigaki/audio/detail/closer.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <hamigaki/binary/binary_io.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/shared_ptr.hpp>
#include <cstring>
#include <utility>

namespace hamigaki { namespace audio {

namespace detail
{

template<typename Source>
class au_file_source_impl
{
    typedef boost::iostreams::stream_offset off_t;

public:
    explicit au_file_source_impl(const Source& src) : src_(src)
    {
        char buf[4*6];
        std::streamsize head_size = static_cast<std::streamsize>(sizeof(buf));

        std::streamsize amt = boost::iostreams::read(src_, buf, head_size);
        if (amt != head_size)
            throw BOOST_IOSTREAMS_FAILURE("cannot read Au header");

        if (std::memcmp(buf, ".snd", 4) != 0)
            throw BOOST_IOSTREAMS_FAILURE("invalid Au header");

        boost::uint32_t offset = decode_uint<big,4>(buf+4);
        boost::uint32_t size = decode_uint<big,4>(buf+8);
        boost::uint32_t encoding = decode_uint<big,4>(buf+12);
        format_.rate = decode_uint<big,4>(buf+16);
        format_.channels = decode_uint<big,4>(buf+20);

        if (offset < sizeof(buf))
            throw BOOST_IOSTREAMS_FAILURE("invalid Au header size");
        else if (offset != sizeof(buf))
        {
            off_t skips = static_cast<off_t>(offset-sizeof(buf));
            boost::iostreams::seek(src_, skips, BOOST_IOS::cur);
        }

        if (size != 0xFFFFFFFF)
            size_ = static_cast<off_t>(size);
        else
            size_ = -1;

        if (encoding == 1)
            format_.type = mu_law;
        else if (encoding == 2)
            format_.type = int8;
        else if (encoding == 3)
            format_.type = int_be16;
        else if (encoding == 4)
            format_.type = int_be24;
        else if (encoding == 5)
            format_.type = int_be32;
        else if (encoding == 6)
            format_.type = float_be32;
        else if (encoding == 7)
            format_.type = float_be64;
        else if (encoding == 27)
            format_.type = a_law;
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported pcm format");

        start_ = static_cast<off_t>(offset);
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
        if (size_ != -1)
        {
            off_t rest = size_ - position_;
            if (rest < static_cast<off_t>(n))
                n = static_cast<std::streamsize>(rest);
        }

        if (n == 0)
            return -1;

        std::streamsize amt = boost::iostreams::read(src_, s, n);
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
            if ((off < 0) || ((size_ != -1) && (off > size_)))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = start_ + off;
        }
        else if (way == BOOST_IOS::cur)
        {
            // Optimization for "tell"
            if (off == 0)
                return position_;

            off_t pos = position_ + off;
            if ((pos < 0) || ((size_ != -1) && (pos > size_)))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
        }
        else if (size_ != -1)
        {
            off_t pos = size_ + off;
            if ((pos < 0) || (pos > size_))
                throw BOOST_IOSTREAMS_FAILURE("bad seek");
            off = pos - size_;
        }

        std::streampos pos = boost::iostreams::seek(src_, off, way);
        position_ = boost::iostreams::position_to_offset(pos) - start_;
        return boost::iostreams::offset_to_position(position_);
    }

private:
    Source src_;
    pcm_format format_;
    off_t size_;
    off_t start_;
    off_t position_;
};

template<typename Sink>
class au_file_sink_impl
{
    typedef boost::iostreams::stream_offset off_t;

public:
    au_file_sink_impl(const Sink& sink, const pcm_format& fmt)
        : sink_(sink), format_(fmt), size_(0)
    {
        char buf[4*6];
        std::streamsize head_size = static_cast<std::streamsize>(sizeof(buf));

        std::memcpy(buf, ".snd", 4);
        encode_uint<big,4>(buf+4, sizeof(buf));
        encode_uint<big,4>(buf+8, 0xFFFFFFFF);
        encode_uint<big,4>(buf+12, to_au_encording(format_.type));
        encode_uint<big,4>(buf+16, format_.rate);
        encode_uint<big,4>(buf+20, format_.channels);

        std::streamsize amt = boost::iostreams::write(sink_, buf, head_size);
        if (amt != head_size)
            throw BOOST_IOSTREAMS_FAILURE("cannot write Au header");
    }

    pcm_format format() const
    {
        return format_;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if (size_ != -1)
        {
            off_t rest = static_cast<off_t>(0xFFFFFFFF) - size_;
            if (rest < static_cast<off_t>(n))
                size_ = -1;
            else
                size_ += n;
        }

        std::streamsize amt = boost::iostreams::write(sink_, s, n);
        if (amt != n)
            throw BOOST_IOSTREAMS_FAILURE("write error");

        return amt;
    }

    void close()
    {
        bool nothrow = false;
        detail::device_closer<Sink> close_sink(sink_, nothrow);

        if (size_ != -1)
        {
            try
            {
                boost::iostreams::seek(sink_, 8, BOOST_IOS::beg);

                char buf[4];
                encode_int<big,4>(&buf[0], size_);
                boost::iostreams::write(sink_, buf, sizeof(buf));
            }
            catch (...)
            {
                nothrow = true;
                throw;
            }
        }
    }

private:
    Sink sink_;
    pcm_format format_;
    off_t size_;

    static boost::uint32_t to_au_encording(sample_format_type type)
    {
        if (type == mu_law)
            return 1;
        else if (type == int8)
            return 2;
        else if (type == int_be16)
            return 3;
        else if (type == int_be24)
            return 4;
        else if (type == int_be32)
            return 5;
        else if (type == float_be32)
            return 6;
        else if (type == float_be64)
            return 7;
        else if (type == a_law)
            return 27;
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported pcm format");
    }
};

} // namespace detail

template<typename Source>
class basic_au_file_source
{
private:
    typedef detail::au_file_source_impl<Source> impl_type;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::optimally_buffered_tag,
        boost::iostreams::input_seekable,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag,
        pcm_format_tag {};

    explicit basic_au_file_source(const Source& src)
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

class au_file_source
{
private:
    typedef basic_au_file_source<iostreams::file_source> impl_type;

public:
    typedef impl_type::char_type char_type;
    typedef impl_type::category category;

    explicit au_file_source(const std::string& path)
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
inline basic_au_file_source<Source> make_au_file_source(const Source& src)
{
    return basic_au_file_source<Source>(src);
}


template<typename Sink>
class basic_au_file_sink
{
private:
    typedef detail::au_file_sink_impl<Sink> impl_type;

public:
    typedef char char_type;

    struct category
        : boost::iostreams::optimally_buffered_tag
        , boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
        , pcm_format_tag
    {};

    basic_au_file_sink(const Sink& sink, const pcm_format& fmt)
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

class au_file_sink
{
private:
    typedef basic_au_file_sink<iostreams::file_sink> impl_type;

public:
    typedef impl_type::char_type char_type;
    typedef impl_type::category category;

    au_file_sink(const std::string& path, const pcm_format& fmt)
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
inline basic_au_file_sink<Sink>
make_au_file_sink(const Sink& sink, const pcm_format& fmt)
{
    return basic_au_file_sink<Sink>(sink, fmt);
}

} } // End namespaces audio, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::audio::basic_au_file_source, 1)

#endif // HAMIGAKI_AUDIO_AU_FILE_HPP
