// asio.hpp: ASIO devices

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_ASIO_HPP
#define HAMIGAKI_AUDIO_ASIO_HPP

#include <hamigaki/audio/detail/config.hpp>
#include <hamigaki/audio/detail/auto_link/hamigaki_audio.hpp>
#include <hamigaki/audio/sample_format.hpp>
#include <hamigaki/uuid.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/shared_ptr.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251)
#endif

#if defined(BOOST_WINDOWS) && !defined(__GNUC__)
#pragma comment(lib, "ole32.lib")
#endif

namespace hamigaki { namespace audio {

struct asio_buffer_info
{
    long min_size;
    long max_size;
    long preferred_size;
    long granularity;
};

class asio_source;
class asio_sink;
class HAMIGAKI_AUDIO_DECL asio_device
{
    friend class asio_source;
    friend class asio_sink;

public:
    class impl;

    explicit asio_device(const uuid& clsid, void* hwnd=0);

    double rate() const;
    void rate(double r);

    void create_buffers(long in_channels, long out_channels);

    asio_source get_source(std::size_t idx);
    asio_sink get_sink(std::size_t idx);

    std::size_t source_channels() const;
    std::size_t sink_channels() const;

    std::streamsize buffer_size() const;
    void buffer_size(std::streamsize n);

    asio_buffer_info buffer_info() const;

private:
    boost::shared_ptr<impl> pimpl_;
};

class HAMIGAKI_AUDIO_DECL asio_source
{
    friend class asio_device::impl;

public:
    class impl;
    typedef char char_type;

    struct category :
        boost::iostreams::source_tag,
        boost::iostreams::closable_tag,
        sample_format_tag {};

    std::streamsize read(char* s, std::streamsize n);
    void close();

    sample_format_type sample_format() const;

private:
    boost::shared_ptr<impl> pimpl_;

    explicit asio_source(const boost::shared_ptr<impl>& pimpl);
};

class HAMIGAKI_AUDIO_DECL asio_sink
{
    friend class asio_device::impl;

public:
    typedef char char_type;

    struct category :
        boost::iostreams::sink_tag,
        boost::iostreams::closable_tag,
        sample_format_tag {};

    std::streamsize write(const char* s, std::streamsize n);
    void close();

    sample_format_type sample_format() const;

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;

    explicit asio_sink(const boost::shared_ptr<impl>& pimpl);
};

} } // End namespaces audio, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_AUDIO_ASIO_HPP
