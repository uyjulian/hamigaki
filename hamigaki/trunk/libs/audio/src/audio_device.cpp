// audio_device.cpp: /dev/audio device

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <boost/config.hpp>
#include <hamigaki/audio/audio_device.hpp>
#include <boost/detail/endian.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/noncopyable.hpp>
#include <stdexcept>

#include <sys/audioio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#if !defined(AUDIO_ENCODING_SLINEAR)
    #define AUDIO_ENCODING_SLINEAR AUDIO_ENCODING_LINEAR
#endif

#if defined(AUDIO_ENCODING_SLINEAR_LE)
    #define HAMIGAKI_AUDIO_ENCODING_SLINEAR_LE AUDIO_ENCODING_SLINEAR_LE
#elif defined(BOOST_LITTLE_ENDIAN)
    #define HAMIGAKI_AUDIO_ENCODING_SLINEAR_LE AUDIO_ENCODING_SLINEAR
#endif

#if defined(AUDIO_ENCODING_SLINEAR_BE)
    #define HAMIGAKI_AUDIO_ENCODING_SLINEAR_BE AUDIO_ENCODING_SLINEAR_BE
#elif defined(BOOST_BIG_ENDIAN)
    #define HAMIGAKI_AUDIO_ENCODING_SLINEAR_BE AUDIO_ENCODING_SLINEAR
#endif

namespace hamigaki { namespace audio {

namespace
{

const char default_path[] = "/dev/audio";

class dev_audio : boost::noncopyable
{
public:
    explicit dev_audio(const char* ph, int mode) :
        fd_(::open(ph, mode))
    {
        if (fd_ == -1)
            throw BOOST_IOSTREAMS_FAILURE("cannot open audio");
    }

    ~dev_audio()
    {
        close();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n > 0)
        {
            int amt = ::write(fd_, s, n);
            if (amt < 0)
                throw BOOST_IOSTREAMS_FAILURE("cannot write audio");
            total += amt;
            s += amt;
            n -= amt;
        }
        return total;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n > 0)
        {
            int amt = ::read(fd_, s, n);
            if (amt < 0)
                throw BOOST_IOSTREAMS_FAILURE("cannot read audio");
            total += amt;
            s += amt;
            n -= amt;
        }
        return (total != 0) ? total : -1;
    }

    void close()
    {
        if (fd_ != -1)
        {
            ::close(fd_);
            fd_ = -1;
        }
    }

    bool flush()
    {
        return ::ioctl(fd_, AUDIO_DRAIN) != -1;
    }

    void info(const audio_info_t& info)
    {
        if (::ioctl(fd_, AUDIO_SETINFO, &info) == -1)
            throw BOOST_IOSTREAMS_FAILURE("cannot change audio format");
    }

private:
    int fd_;
};

void make_audio_prinfo(audio_prinfo_t& prinfo, const pcm_format& f)
{
    if (f.type == mu_law)
    {
        prinfo.encoding = AUDIO_ENCODING_ULAW;
        prinfo.precision = 8;
    }
    else if (f.type == a_law)
    {
        prinfo.encoding = AUDIO_ENCODING_ALAW;
        prinfo.precision = 8;
    }
    else if (f.type == uint8)
    {
        prinfo.encoding = AUDIO_ENCODING_LINEAR8;
        prinfo.precision = 8;
    }
    else if (f.type == int8)
    {
        prinfo.encoding = AUDIO_ENCODING_LINEAR;
        prinfo.precision = 8;
    }
#if defined(HAMIGAKI_AUDIO_ENCODING_SLINEAR_LE)
    else if (f.type == int_le16)
    {
        prinfo.encoding = HAMIGAKI_AUDIO_ENCODING_SLINEAR_LE;
        prinfo.precision = 16;
    }
#endif
#if defined(HAMIGAKI_AUDIO_ENCODING_SLINEAR_BE)
    else if (f.type == int_be16)
    {
        prinfo.encoding = HAMIGAKI_AUDIO_ENCODING_SLINEAR_BE;
        prinfo.precision = 16;
    }
#endif
    else
        throw BOOST_IOSTREAMS_FAILURE("unsupported audio format");

    prinfo.sample_rate = f.rate;
    prinfo.channels = f.channels;
}

} // namespace

class audio_sink::impl : public dev_audio
{
public:
    impl(const char* ph, const pcm_format& f)
        : dev_audio(ph, O_WRONLY), format_(f)
    {
        audio_info_t info;
        AUDIO_INITINFO(&info);

        make_audio_prinfo(info.play, f);
        this->info(info);
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    pcm_format format_;
};

class audio_source::impl : public dev_audio
{
public:
    impl(const char* ph, const pcm_format& f)
        : dev_audio(ph, O_RDONLY), format_(f)
    {
        audio_info_t info;
        AUDIO_INITINFO(&info);

        make_audio_prinfo(info.record, f);
        this->info(info);
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    pcm_format format_;
};

audio_sink::audio_sink(const pcm_format& f)
    : pimpl_(new impl(default_path, f))
{
}

audio_sink::audio_sink(const char* ph, const pcm_format& f)
    : pimpl_(new impl(ph, f))
{
}

pcm_format audio_sink::format() const
{
    return pimpl_->format();
}

std::streamsize audio_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

bool audio_sink::flush()
{
    return pimpl_->flush();
}

void audio_sink::close()
{
    pimpl_->close();
}

audio_source::audio_source(const pcm_format& f)
    : pimpl_(new impl(default_path, f))
{
}

audio_source::audio_source(const char* ph, const pcm_format& f)
    : pimpl_(new impl(ph, f))
{
}

pcm_format audio_source::format() const
{
    return pimpl_->format();
}

std::streamsize audio_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void audio_source::close()
{
    pimpl_->close();
}

} } // End namespaces audio, hamigaki.
