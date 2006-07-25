//  pcm_device.hpp: a simple pcm device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_AUDIO_PCM_DEVICE_HPP
#define HAMIGAKI_AUDIO_PCM_DEVICE_HPP

#include <hamigaki/audio/detail/config.hpp>
#include <hamigaki/audio/detail/auto_link/hamigaki_audio.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/shared_ptr.hpp>

#include <boost/config/abi_prefix.hpp>

#if defined(BOOST_WINDOWS) && !defined(__GNUC__)
#pragma comment(lib, "winmm.lib")
#endif

namespace hamigaki { namespace audio {

class HAMIGAKI_AUDIO_DECL pcm_sink
{
public:
    typedef char char_type;

    struct category :
        boost::iostreams::sink_tag,
        boost::iostreams::closable_tag,
        boost::iostreams::flushable_tag,
        boost::iostreams::optimally_buffered_tag,
        pcm_format_tag {};

    explicit pcm_sink(const pcm_format& f);
    pcm_sink(const pcm_format& f, std::size_t buffer_size);

    std::streamsize optimal_buffer_size() const
    {
        return this->format().optimal_buffer_size();
    }

    pcm_format format() const;

    std::streamsize write(const char* s, std::streamsize n);
    bool flush();
    void close();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

class HAMIGAKI_AUDIO_DECL pcm_source
{
public:
    typedef char char_type;

    struct category :
        boost::iostreams::source_tag,
        boost::iostreams::closable_tag,
        boost::iostreams::optimally_buffered_tag,
        pcm_format_tag {};

    explicit pcm_source(const pcm_format& f);
    pcm_source(const pcm_format& f, std::size_t buffer_size);

    std::streamsize optimal_buffer_size() const
    {
        return this->format().optimal_buffer_size();
    }

    pcm_format format() const;

    std::streamsize read(char* s, std::streamsize n);
    void close();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

} } // End namespaces audio, hamigaki.

#include <boost/config/abi_suffix.hpp>

#endif // HAMIGAKI_AUDIO_PCM_DEVICE_HPP
