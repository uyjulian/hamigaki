// direct_sound.hpp: DirectSound device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DIRECT_SOUND_HPP
#define HAMIGAKI_AUDIO_DIRECT_SOUND_HPP

#include <hamigaki/audio/detail/config.hpp>
#include <hamigaki/audio/detail/auto_link/hamigaki_audio.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <hamigaki/coroutine/generator.hpp>
#include <hamigaki/uuid.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <utility>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251 4275)
#endif

#if defined(BOOST_WINDOWS) && !defined(__GNUC__)
#pragma comment(lib, "user32.lib")
#endif

struct IDirectSoundBuffer;
struct IDirectSoundCaptureBuffer;

namespace hamigaki { namespace audio {

namespace direct_sound
{

HAMIGAKI_AUDIO_DECL extern const unsigned long normal_level;
HAMIGAKI_AUDIO_DECL extern const unsigned long priority_level;
HAMIGAKI_AUDIO_DECL extern const unsigned long exclusive_level;
HAMIGAKI_AUDIO_DECL extern const unsigned long write_primary_level;

struct device_info
{
    uuid driver_guid;
    std::string description;
    std::string module_name;
};

typedef hamigaki::coroutines::generator<device_info> device_info_iterator;

namespace detail
{

HAMIGAKI_AUDIO_DECL device_info enum_devices(device_info_iterator::self& self);

HAMIGAKI_AUDIO_DECL
device_info enum_capture_devices(device_info_iterator::self& self);

} // namespace detail

inline std::pair<device_info_iterator,device_info_iterator>
device_info_range()
{
    return std::pair<device_info_iterator,device_info_iterator>(
        device_info_iterator(detail::enum_devices),
        device_info_iterator()
    );
}

inline std::pair<device_info_iterator,device_info_iterator>
capture_device_info_range()
{
    return std::pair<device_info_iterator,device_info_iterator>(
        device_info_iterator(detail::enum_capture_devices),
        device_info_iterator()
    );
}

} // namespace direct_sound

class HAMIGAKI_AUDIO_DECL direct_sound_error : public BOOST_IOSTREAMS_FAILURE
{
public:
    explicit direct_sound_error(long error);
    long error() const { return error_; }
    static void check(long error);

private:
    long error_;
};

class HAMIGAKI_AUDIO_DECL direct_sound_sink
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::sink_tag
        , boost::iostreams::closable_tag
        , boost::iostreams::optimally_buffered_tag
        , pcm_format_tag
    {};

    direct_sound_sink(::IDirectSoundBuffer* p,
        const pcm_format& f, std::size_t buffer_size);

    pcm_format format() const;

    std::streamsize optimal_buffer_size() const
    {
        return this->format().optimal_buffer_size();
    }

    std::streamsize write(const char* s, std::streamsize n);
    void close();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

class HAMIGAKI_AUDIO_DECL direct_sound_device
{
public:
    direct_sound_device();
    explicit direct_sound_device(const uuid& driver_guid);

    void set_cooperative_level(void* hwnd, unsigned long level);

    void format(const pcm_format& f);

    direct_sound_sink create_buffer(
        const pcm_format& f, std::size_t buffer_size);

    direct_sound_sink create_buffer(const pcm_format& f);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

class HAMIGAKI_AUDIO_DECL direct_sound_source
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::source_tag
        , boost::iostreams::closable_tag
        , boost::iostreams::optimally_buffered_tag
        , pcm_format_tag
    {};

    direct_sound_source(::IDirectSoundCaptureBuffer* p,
        const pcm_format& f, std::size_t buffer_size);

    pcm_format format() const;

    std::streamsize optimal_buffer_size() const
    {
        return this->format().optimal_buffer_size();
    }

    std::streamsize read(char* s, std::streamsize n);
    void close();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

class HAMIGAKI_AUDIO_DECL direct_sound_capture
{
public:
    direct_sound_capture();
    explicit direct_sound_capture(const uuid& driver_guid);

    direct_sound_source create_buffer(
        const pcm_format& f, std::size_t buffer_size);

    direct_sound_source create_buffer(const pcm_format& f);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

} } // End namespaces audio, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_AUDIO_DIRECT_SOUND_HPP
