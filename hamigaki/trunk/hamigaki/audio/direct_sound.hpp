//  direct_sound.hpp: DirectSound device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hamigaki/audio/detail/config.hpp>
#include <hamigaki/audio/detail/auto_link/hamigaki_audio.hpp>
#include <hamigaki/audio/pcm_format.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include <string>

#if defined(BOOST_WINDOWS) && !defined(__GNUC__)
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
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
    std::string driver_guid;
    std::string description;
    std::string module_name;
};

} // namespace direct_sound

namespace detail
{

class ds_enum_callback_base
{
public:
    virtual ~ds_enum_callback_base(){}

    bool next(const direct_sound::device_info& info)
    {
        return do_next(info);
    }

private:
    virtual bool do_next(const direct_sound::device_info& info) = 0;
};

template<class Function>
struct ds_enum_callback : ds_enum_callback_base
{
    explicit ds_enum_callback(Function f) : func_(f)
    {
    }

    Function func_;

private:
    bool do_next(const direct_sound::device_info& info) // virtual
    {
        return static_cast<bool>(func_(info));
    }
};

HAMIGAKI_AUDIO_DECL
void direct_sound_enumerate_impl(ds_enum_callback_base* ptr);

template<class OutputIterator>
struct copy_functor
{
    explicit copy_functor(OutputIterator out) : out_(out) {}

    bool operator()(const direct_sound::device_info& info)
    {
        out_ = info;
        ++out_;
        return true;
    }

    OutputIterator out_;
};

template<class Predicate>
struct find_functor
{
    explicit find_functor(Predicate pred) : pred_(pred) {}

    bool operator()(const direct_sound::device_info& info)
    {
        if (pred_(info))
        {
            info_ = info;
            return false;
        }
        else
            return true;
    }

    Predicate pred_;
    boost::optional<direct_sound::device_info> info_;
};

} // namespace detail

class HAMIGAKI_AUDIO_DECL direct_sound_error : public BOOST_IOSTREAMS_FAILURE
{
public:
    explicit direct_sound_error(long error);
    long error() const { return error_; }
    static void check(long error);

private:
    long error_;
};

template<class Function>
inline Function direct_sound_enumerate(Function f)
{
    detail::ds_enum_callback<Function> callback(f);
    detail::direct_sound_enumerate_impl(&callback);
    return callback.func_;
}

template<class OutputIterator>
inline OutputIterator direct_sound_enumerate_copy(OutputIterator result)
{
    return direct_sound_enumerate(
        detail::copy_functor<OutputIterator>(result)).out_;
}

template<class Predicate>
inline boost::optional<direct_sound::device_info>
direct_sound_find_if(Predicate pred)
{
    return direct_sound_enumerate(
        detail::find_functor<Predicate>(pred)).info_;
}

class HAMIGAKI_AUDIO_DECL direct_sound_buffer
{
public:
    typedef char char_type;

    struct category :
        boost::iostreams::sink_tag,
        boost::iostreams::closable_tag,
        boost::iostreams::optimally_buffered_tag,
        pcm_format_tag {};

    direct_sound_buffer(::IDirectSoundBuffer* p,
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
    explicit direct_sound_device(const std::string& guid_str);

    void set_cooperative_level(void* hwnd, unsigned long level);

    void format(const pcm_format& f);

    direct_sound_buffer create_buffer(
        const pcm_format& f, std::size_t buffer_size);

    direct_sound_buffer create_buffer(const pcm_format& f);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

class HAMIGAKI_AUDIO_DECL direct_sound_capture_buffer
{
public:
    typedef char char_type;

    struct category :
        boost::iostreams::source_tag,
        boost::iostreams::closable_tag,
        boost::iostreams::optimally_buffered_tag,
        pcm_format_tag {};

    direct_sound_capture_buffer(::IDirectSoundCaptureBuffer* p,
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
    explicit direct_sound_capture(const std::string& guid_str);

    direct_sound_capture_buffer create_buffer(
        const pcm_format& f, std::size_t buffer_size);

    direct_sound_capture_buffer create_buffer(const pcm_format& f);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

} } // End namespaces audio, hamigaki.
