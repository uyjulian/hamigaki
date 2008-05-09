// pulse_audio.cpp: PulseAudio device

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/pulse_audio.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <pulse/simple.h>

namespace
{

pa_sample_format_t type_to_format(hamigaki::audio::sample_format_type type)
{
    if (type == hamigaki::audio::uint8)
        return PA_SAMPLE_U8;
    else if (type == hamigaki::audio::int_le16)
        return PA_SAMPLE_S16LE;
    else if (type == hamigaki::audio::int_be16)
        return PA_SAMPLE_S16BE;
    else if (type == hamigaki::audio::float_le32)
        return PA_SAMPLE_FLOAT32LE;
    else if (type == hamigaki::audio::float_be32)
        return PA_SAMPLE_FLOAT32BE;
    else
        throw BOOST_IOSTREAMS_FAILURE("unsupported PCM format");
}

} // namespace

namespace hamigaki { namespace audio {

class pulse_audio_sink::impl
{
public:
    impl(const char* app, const char* name, const pcm_format& fmt) : fmt_(fmt)
    {
        pa_sample_spec ss = {};
        ss.format = type_to_format(fmt.type);
        ss.rate = fmt.rate;
        ss.channels = fmt.channels;

        int error;
        handle_ = ::pa_simple_new(
            0, app, PA_STREAM_PLAYBACK, 0, name, &ss, 0, 0, &error);
        if (handle_ == 0)
            throw BOOST_IOSTREAMS_FAILURE("pa_simple_new() failed");
    }

    ~impl()
    {
        ::pa_simple_free(handle_);
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        int error;
        if (::pa_simple_write(handle_, s, n, &error) < 0)
            throw BOOST_IOSTREAMS_FAILURE("pa_simple_write() failed");

        return n;
    }

    pcm_format format() const
    {
        return fmt_;
    }

    void close()
    {
        int error;
        if (::pa_simple_drain(handle_, &error) < 0)
            throw BOOST_IOSTREAMS_FAILURE("pa_simple_write() failed");
    }

private:
    pa_simple* handle_;
    pcm_format fmt_;
};

pulse_audio_sink::pulse_audio_sink(
    const char* app, const char* name, const pcm_format& fmt)
    : pimpl_(new impl(app, name, fmt))
{
}

std::streamsize pulse_audio_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

pcm_format pulse_audio_sink::format() const
{
    return pimpl_->format();
}

void pulse_audio_sink::close()
{
    pimpl_->close();
}


class pulse_audio_source::impl
{
public:
    impl(const char* app, const char* name, const pcm_format& fmt) : fmt_(fmt)
    {
        pa_sample_spec ss = {};
        ss.format = type_to_format(fmt.type);
        ss.rate = fmt.rate;
        ss.channels = fmt.channels;

        int error;
        handle_ = ::pa_simple_new(
            0, app, PA_STREAM_RECORD, 0, name, &ss, 0, 0, &error);
        if (handle_ == 0)
            throw BOOST_IOSTREAMS_FAILURE("pa_simple_new() failed");
    }

    ~impl()
    {
        ::pa_simple_free(handle_);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        int error;
        if (::pa_simple_read(handle_, s, n, &error) < 0)
            throw BOOST_IOSTREAMS_FAILURE("pa_simple_read() failed");

        return n;
    }

    pcm_format format() const
    {
        return fmt_;
    }

    void close()
    {
    }

private:
    pa_simple* handle_;
    pcm_format fmt_;
};

pulse_audio_source::pulse_audio_source(
    const char* app, const char* name, const pcm_format& fmt)
    : pimpl_(new impl(app, name, fmt))
{
}

std::streamsize pulse_audio_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

pcm_format pulse_audio_source::format() const
{
    return pimpl_->format();
}

void pulse_audio_source::close()
{
    pimpl_->close();
}

} } // End namespaces audio, hamigaki.
