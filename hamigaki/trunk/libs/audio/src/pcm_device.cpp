// pcm_device.cpp: a simple pcm device

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <boost/config.hpp>
#include <hamigaki/audio/pcm_device.hpp>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/audio/mme_device.hpp>
#elif defined(__APPLE__)
    #include <hamigaki/audio/audio_unit.hpp>
#elif defined(__NetBSD__)||defined(__OpenBSD__)||defined(sun)||defined(__sun)
    #include <hamigaki/audio/audio_device.hpp>
    #define HAMIGAKI_AUDIO_USE_DEV_AUDIO
#else
    #include <hamigaki/audio/dsp_device.hpp>
#endif

namespace hamigaki { namespace audio {

#if defined(BOOST_WINDOWS)
class pcm_sink::impl
{
public:
    impl(const pcm_format& f, std::size_t buffer_size) : impl_(f, buffer_size)
    {
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return impl_.write(s, n);
    }

    bool flush()
    {
        return impl_.flush();
    }

    void close()
    {
        impl_.close();
    }

private:
    mme_sink impl_;
};

class pcm_source::impl
{
public:
    impl(const pcm_format& f, std::size_t buffer_size) : impl_(f, buffer_size)
    {
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

    void close()
    {
        impl_.close();
    }

private:
    mme_source impl_;
};

#elif defined(__APPLE__) // not defined(BOOST_WINDOWS) and defined(__APPLE__)
class pcm_sink::impl
{
public:
    impl(const pcm_format& f, std::size_t buffer_size) : impl_(f, buffer_size)
    {
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return impl_.write(s, n);
    }

    bool flush()
    {
        return impl_.flush();
    }

    void close()
    {
        impl_.close();
    }

private:
    audio_unit_sink impl_;
};

class pcm_source::impl
{
public:
    impl(const pcm_format& f, std::size_t buffer_size) : impl_(f, buffer_size)
    {
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

    void close()
    {
        impl_.close();
    }

private:
    audio_unit_source impl_;
};

#elif defined(HAMIGAKI_AUDIO_USE_DEV_AUDIO)
class pcm_sink::impl
{
public:
    impl(const pcm_format& f, std::size_t) : impl_(f)
    {
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return impl_.write(s, n);
    }

    bool flush()
    {
        return impl_.flush();
    }

    void close()
    {
        impl_.close();
    }

private:
    audio_sink impl_;
};

class pcm_source::impl
{
public:
    impl(const pcm_format& f, std::size_t) : impl_(f)
    {
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

    void close()
    {
        impl_.close();
    }

private:
    audio_source impl_;
};

#else // use /dev/dsp
class pcm_sink::impl
{
public:
    impl(const pcm_format& f, std::size_t) : impl_(f)
    {
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        return impl_.write(s, n);
    }

    bool flush()
    {
        return impl_.flush();
    }

    void close()
    {
        impl_.close();
    }

private:
    dsp_sink impl_;
};

class pcm_source::impl
{
public:
    impl(const pcm_format& f, std::size_t) : impl_(f)
    {
    }

    pcm_format format() const
    {
        return impl_.format();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return impl_.read(s, n);
    }

    void close()
    {
        impl_.close();
    }

private:
    dsp_source impl_;
};
#endif // not defined(BOOST_WINDOWS) and not defined(__APPLE__)

pcm_sink::pcm_sink(const pcm_format& f)
    : pimpl_(new impl(f, f.optimal_buffer_size()))
{
}

pcm_sink::pcm_sink(const pcm_format& f, std::size_t buffer_size)
    : pimpl_(new impl(f, buffer_size))
{
}

pcm_format pcm_sink::format() const
{
    return pimpl_->format();
}

std::streamsize pcm_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

bool pcm_sink::flush()
{
    return pimpl_->flush();
}

void pcm_sink::close()
{
    pimpl_->close();
}

pcm_source::pcm_source(const pcm_format& f)
    : pimpl_(new impl(f, f.optimal_buffer_size()))
{
}

pcm_source::pcm_source(const pcm_format& f, std::size_t buffer_size)
    : pimpl_(new impl(f, buffer_size))
{
}

pcm_format pcm_source::format() const
{
    return pimpl_->format();
}

std::streamsize pcm_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void pcm_source::close()
{
    pimpl_->close();
}

} } // End namespaces audio, hamigaki.
