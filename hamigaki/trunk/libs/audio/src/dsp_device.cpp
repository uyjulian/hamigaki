// dsp_device.cpp: /dev/dsp device

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/dsp_device.hpp>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/noncopyable.hpp>
#include <stdexcept>

#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <fcntl.h>
#include <unistd.h>

namespace hamigaki { namespace audio {

namespace
{

const char default_path[] = "/dev/dsp";

class dev_dsp : boost::noncopyable
{
public:
    explicit dev_dsp(const char* ph, int mode) :
        fd_(::open(ph, mode))
    {
        if (fd_ == -1)
            throw BOOST_IOSTREAMS_FAILURE("cannot open DSP");
    }

    ~dev_dsp()
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
                throw BOOST_IOSTREAMS_FAILURE("cannot write DSP");
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
                throw BOOST_IOSTREAMS_FAILURE("cannot read DSP");
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
        return true;
    }

    void format(int value)
    {
        int tmp = value;
        if (::ioctl(fd_, SNDCTL_DSP_SETFMT, &tmp) == -1)
            throw BOOST_IOSTREAMS_FAILURE("cannot change DSP format");
        if (tmp != value)
            throw BOOST_IOSTREAMS_FAILURE("unsupported DSP format");
    }

    void channels(int value)
    {
        int tmp = value;
        if (::ioctl(fd_, SNDCTL_DSP_CHANNELS, &tmp) == -1)
            throw BOOST_IOSTREAMS_FAILURE("cannot change DSP channels");
        if (tmp != value)
            throw BOOST_IOSTREAMS_FAILURE("unsupported DSP channels");
    }

    void speed(int value)
    {
        int tmp = value;
        if (::ioctl(fd_, SNDCTL_DSP_SPEED, &tmp) == -1)
            throw BOOST_IOSTREAMS_FAILURE("cannot change DSP speed");
        if (tmp != value)
            throw BOOST_IOSTREAMS_FAILURE("unsupported DSP speed");
    }

private:
    int fd_;
};

} // namespace

class dsp_sink::impl : public dev_dsp
{
public:
    impl(const char* ph, const pcm_format& f)
        : dev_dsp(ph, O_WRONLY), format_(f)
    {
        if (f.type == uint8)
            dev_dsp::format(AFMT_U8);
        else if (f.type == int_le16)
            dev_dsp::format(AFMT_S16_LE);
        else if (f.type == mu_law)
            dev_dsp::format(AFMT_MU_LAW);
        else if (f.type == a_law)
            dev_dsp::format(AFMT_A_LAW);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported DSP format");
        dev_dsp::channels(f.channels);
        dev_dsp::speed(f.rate);
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    pcm_format format_;
};

class dsp_source::impl : public dev_dsp
{
public:
    impl(const char* ph, const pcm_format& f)
        : dev_dsp(ph, O_RDONLY), format_(f)
    {
        if (f.type == uint8)
            dev_dsp::format(AFMT_U8);
        else if (f.type == int_le16)
            dev_dsp::format(AFMT_S16_LE);
        else if (f.type == mu_law)
            dev_dsp::format(AFMT_MU_LAW);
        else if (f.type == a_law)
            dev_dsp::format(AFMT_A_LAW);
        else
            throw BOOST_IOSTREAMS_FAILURE("unsupported DSP format");
        dev_dsp::channels(f.channels);
        dev_dsp::speed(f.rate);
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    pcm_format format_;
};

dsp_sink::dsp_sink(const pcm_format& f)
    : pimpl_(new impl(default_path, f))
{
}

dsp_sink::dsp_sink(const char* ph, const pcm_format& f)
    : pimpl_(new impl(ph, f))
{
}

pcm_format dsp_sink::format() const
{
    return pimpl_->format();
}

std::streamsize dsp_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

bool dsp_sink::flush()
{
    return pimpl_->flush();
}

void dsp_sink::close()
{
    pimpl_->close();
}

dsp_source::dsp_source(const pcm_format& f)
    : pimpl_(new impl(default_path, f))
{
}

dsp_source::dsp_source(const char* ph, const pcm_format& f)
    : pimpl_(new impl(ph, f))
{
}

pcm_format dsp_source::format() const
{
    return pimpl_->format();
}

std::streamsize dsp_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void dsp_source::close()
{
    pimpl_->close();
}

} } // End namespaces audio, hamigaki.
