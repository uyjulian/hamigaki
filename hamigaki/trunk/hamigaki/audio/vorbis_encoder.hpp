// vorbis_encoder.hpp: vorbisenc device adaptor

// Copyright Takeshi Mouri 2006-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_VORBIS_ENCODER_HPP
#define HAMIGAKI_AUDIO_VORBIS_ENCODER_HPP

#include <hamigaki/audio/detail/config.hpp>
#include <hamigaki/audio/detail/auto_link/hamigaki_audio.hpp>
#include <hamigaki/audio/detail/auto_link/ogg.hpp>
#include <hamigaki/audio/detail/auto_link/vorbis.hpp>
#include <hamigaki/audio/detail/auto_link/vorbisenc.hpp>
#include <hamigaki/audio/detail/auto_link/vorbisfile.hpp>
#include <hamigaki/audio/detail/closer.hpp>
#include <hamigaki/iostreams/device/file.hpp>
#include <hamigaki/iostreams/arbitrary_positional_facade.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/noncopyable.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4275)
#endif

namespace hamigaki { namespace audio {

struct vorbis_encode_params
{
    long max_bitrate;
    long nominal_bitrate;
    long min_bitrate;
};

namespace detail
{

HAMIGAKI_AUDIO_DECL int random_serial_no();

class HAMIGAKI_AUDIO_DECL vorbis_encoder_base
    : public iostreams::arbitrary_positional_facade<
        vorbis_encoder_base, float, 255
    >
    , private boost::noncopyable
{
    friend class iostreams::core_access;

public:
    typedef std::streamsize (*write_func)(void*, const char*, std::streamsize);
    typedef void (*close_func)(void*);

    vorbis_encoder_base();
    ~vorbis_encoder_base();

    long channels() const;
    long rate() const;

    void open(void* self, long channels, long rate, float quality, int serialno,
        write_func write, close_func close);

    void open(void* self, long channels, long rate, int serialno,
        const vorbis_encode_params& params,
        write_func write, close_func close);

    void close();

private:
    void* ptr_;
    bool is_open_;

    std::streamsize write_blocks(const float* s, std::streamsize n);

#if BOOST_WORKAROUND(_MSC_VER, <= 1600)
    std::streamsize read_blocks(float* s, std::streamsize n)
    {
        BOOST_ASSERT(false);
        return -1;
    }

    void close_with_flush(const float* s, std::streamsize n)
    {
        BOOST_ASSERT(false);
    }

    std::streampos seek_blocks(
        boost::iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        BOOST_ASSERT(false);
        return -1;
    }
#endif
};

template<typename Sink>
class vorbis_file_sink_impl
{
public:
    typedef float char_type;

    vorbis_file_sink_impl(
            const Sink& sink, long channels, long rate,
            float quality, int serialno)
        : sink_(sink)
    {
        base_.open(&sink_, channels, rate, quality, serialno,
            &vorbis_file_sink_impl::write_func,
            &vorbis_file_sink_impl::close_func);
    }

    vorbis_file_sink_impl(
            const Sink& sink, long channels, long rate, int serialno,
            const vorbis_encode_params& params)
        : sink_(sink)
    {
        base_.open(&sink_, channels, rate, serialno, params,
            &vorbis_file_sink_impl::write_func,
            &vorbis_file_sink_impl::close_func);
    }

    void close()
    {
        bool nothrow = false;
        detail::device_closer<Sink> close_sink(sink_, nothrow);
        detail::device_closer<vorbis_encoder_base> close_base(base_, nothrow);
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return base_.write(s, n);
    }

    long channels() const
    {
        return base_.channels();
    }

    long rate() const
    {
        return base_.rate();
    }

private:
    vorbis_encoder_base base_;
    Sink sink_;

    static std::streamsize write_func(
        void* datasink, const char* s, std::streamsize n)
    {
        Sink& sink = *static_cast<Sink*>(datasink);
        return boost::iostreams::write(sink, s, n);
    }

    static void close_func(void* datasink)
    {
        Sink& sink = *static_cast<Sink*>(datasink);
        boost::iostreams::close(sink, BOOST_IOS::out);
    }
};

} // namespace detail

template<typename Sink>
class basic_vorbis_file_sink
{
private:
    typedef detail::vorbis_file_sink_impl<Sink> impl_type;

public:
    typedef float char_type;

    struct category
        : boost::iostreams::optimally_buffered_tag
        , boost::iostreams::output
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
    {};

    basic_vorbis_file_sink(
            const Sink& sink, long channels, long rate, float quality=0.1f)
        : pimpl_(new impl_type(
            sink, channels, rate, quality, detail::random_serial_no()))
    {
    }

    basic_vorbis_file_sink(
            const Sink& sink, long channels, long rate,
            float quality, int serialno)
        : pimpl_(new impl_type(sink, channels, rate, quality, serialno))
    {
    }

    basic_vorbis_file_sink(
            const Sink& sink, long channels, long rate,
            const vorbis_encode_params& params)
        : pimpl_(new impl_type(sink, channels, rate,
            detail::random_serial_no(), params))
    {
    }

    basic_vorbis_file_sink(
            const Sink& sink, long channels, long rate, int serialno,
            const vorbis_encode_params& params)
        : pimpl_(new impl_type(sink, channels, rate, serialno, params))
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        return pimpl_->channels() * (pimpl_->rate() / 5);
    }

    long channels() const
    {
        return pimpl_->channels();
    }

    long rate() const
    {
        return pimpl_->rate();
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

class vorbis_file_sink
{
private:
    typedef basic_vorbis_file_sink<iostreams::file_sink> impl_type;

public:
    typedef impl_type::char_type char_type;
    typedef impl_type::category category;

    vorbis_file_sink(const std::string& path,
            long channels, long rate, float quality=0.1f)
        : impl_(iostreams::file_sink(
            path, BOOST_IOS::out|BOOST_IOS::binary), channels, rate, quality)
    {
    }

    vorbis_file_sink(const std::string& path,
            long channels, long rate, float quality, int serialno)
        : impl_(iostreams::file_sink(
            path, BOOST_IOS::out|BOOST_IOS::binary),
            channels, rate, quality, serialno)
    {
    }

    vorbis_file_sink(const std::string& path,
            long channels, long rate, const vorbis_encode_params& params)
        : impl_(iostreams::file_sink(
            path, BOOST_IOS::out|BOOST_IOS::binary), channels, rate, params)
    {
    }

    vorbis_file_sink(const std::string& path,
            long channels, long rate, int serialno,
            const vorbis_encode_params& params)
        : impl_(iostreams::file_sink(
            path, BOOST_IOS::out|BOOST_IOS::binary),
            channels, rate, serialno, params)
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        return impl_.optimal_buffer_size();
    }

    long channels() const
    {
        return impl_.channels();
    }

    long rate() const
    {
        return impl_.rate();
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
inline basic_vorbis_file_sink<Sink>
make_vorbis_file_sink(
    const Sink& sink, long channels, long rate, float quality=0.1f)
{
    return basic_vorbis_file_sink<Sink>(sink, channels, rate, quality);
}

template<typename Sink>
inline basic_vorbis_file_sink<Sink>
make_vorbis_file_sink(
    const Sink& sink, long channels, long rate, float quality, int serialno)
{
    return
        basic_vorbis_file_sink<Sink>(sink, channels, rate, quality, serialno);
}

template<typename Sink>
inline basic_vorbis_file_sink<Sink>
make_vorbis_file_sink(const Sink& sink,
    long channels, long rate, const vorbis_encode_params& params)
{
    return basic_vorbis_file_sink<Sink>(sink, channels, rate, params);
}

template<typename Sink>
inline basic_vorbis_file_sink<Sink>
make_vorbis_file_sink(const Sink& sink,
    long channels, long rate, int serialno, const vorbis_encode_params& params)
{
    return basic_vorbis_file_sink<Sink>(sink, channels, rate, serialno, params);
}

} } // End namespaces audio, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_AUDIO_VORBIS_ENCODER_HPP
