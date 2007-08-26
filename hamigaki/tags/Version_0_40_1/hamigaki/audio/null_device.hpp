// null_device.hpp: null audio device

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_NULL_DEVICE_HPP
#define HAMIGAKI_AUDIO_NULL_DEVICE_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/cstdint.hpp>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long);
#else
    #include <sys/select.h>
#endif

namespace hamigaki { namespace audio {

class null_sink
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::sink_tag
        , boost::iostreams::closable_tag
        , boost::iostreams::optimally_buffered_tag
        , pcm_format_tag
    {};

    explicit null_sink(const pcm_format& f) : format_(f)
    {
    }

    std::streamsize optimal_buffer_size() const
    {
        return format_.optimal_buffer_size();
    }

    pcm_format format() const
    {
        return format_;
    }

    std::streamsize write(const char*, std::streamsize n)
    {
#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
        boost::uint64_t tmp = n / format_.block_size();
        tmp *= 1000;
        tmp /= format_.rate;
        ::Sleep(tmp);
#else
        boost::uint64_t tmp = n / format_.block_size();
        tmp *= 1000000;
        tmp /= format_.rate;

        timeval tv;
        tv.tv_sec = tmp / 1000000u;
        tv.tv_usec = tmp % 1000000u;
        ::select(0, 0, 0, 0, &tv);
#endif
        return n;
    }

    void close()
    {
    }

private:
    pcm_format format_;
};

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_NULL_DEVICE_HPP
