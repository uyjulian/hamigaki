// direct_sound.cpp: DirectSound device

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/direct_sound.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <cctype>
#include <stdexcept>

#include "detail/direct_sound_notify.hpp"
#include "detail/wave_format_ex.hpp"
#include <hamigaki/detail/windows/auto_reset_event.hpp>
#include <hamigaki/detail/windows/com_release.hpp>
#include <hamigaki/detail/windows/dynamic_link_library.hpp>
#include <dsound.h>
#include <windows.h>

using namespace hamigaki::detail::windows;

namespace hamigaki { namespace audio {

namespace direct_sound
{

const unsigned long normal_level = DSSCL_NORMAL;
const unsigned long priority_level = DSSCL_PRIORITY;
const unsigned long exclusive_level = DSSCL_EXCLUSIVE;
const unsigned long write_primary_level = DSSCL_WRITEPRIMARY;

namespace detail
{

namespace
{

::BOOL CALLBACK enum_devices_callback(
    ::GUID* lpGuid, const char* lpcstrDescription,
    const char* lpcstrModule, void* lpContext)
{
    try
    {
        device_info_iterator::self& self =
            *reinterpret_cast<device_info_iterator::self*>(lpContext);

        device_info info;
        info.driver_guid = lpGuid ? uuid(*lpGuid) : uuid();
        info.description = lpcstrDescription;
        info.module_name = lpcstrModule;

        self.yield(info);

        return TRUE;
    }
    catch (...)
    {
    }

    return FALSE;
}

} // namespace

HAMIGAKI_AUDIO_DECL device_info enum_devices(device_info_iterator::self& self)
{
#if !defined(HAMIGAKI_AUDIO_NO_DS_ENUM)
    ::DirectSoundEnumerateA(&enum_devices_callback, &self);
#else
    typedef HRESULT (WINAPI *DirectSoundEnumerateFuncPtr)(
        LPDSENUMCALLBACK lpDSEnumCallback,
        LPVOID lpContext
    );

    dynamic_link_library dsound("dsound.dll");
    DirectSoundEnumerateFuncPtr func_ptr =
        reinterpret_cast<DirectSoundEnumerateFuncPtr>(
            dsound.get_proc_address("DirectSoundEnumerateA"));
    (*func_ptr)(&enum_devices_callback, &self);
#endif
    self.exit();
    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(device_info())
}

HAMIGAKI_AUDIO_DECL
device_info enum_capture_devices(device_info_iterator::self& self)
{
#if !defined(HAMIGAKI_AUDIO_NO_DS_ENUM)
    ::DirectSoundCaptureEnumerateA(&enum_devices_callback, &self);
#else
    typedef HRESULT (WINAPI *DirectSoundEnumerateFuncPtr)(
        LPDSENUMCALLBACK lpDSEnumCallback,
        LPVOID lpContext
    );

    dynamic_link_library dsound("dsound.dll");
    DirectSoundEnumerateFuncPtr func_ptr =
        reinterpret_cast<DirectSoundEnumerateFuncPtr>(
            dsound.get_proc_address("DirectSoundCaptureEnumerateA"));
    (*func_ptr)(&enum_devices_callback, &self);
#endif
    self.exit();
    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(device_info())
}

} // namespace detail

} // namespace direct_sound

direct_sound_error::direct_sound_error(long error)
    : BOOST_IOSTREAMS_FAILURE("DirectSound error"), error_(error) 
{
}

void direct_sound_error::check(long error)
{
    if (FAILED(error))
        throw direct_sound_error(error);
}

namespace
{

const std::size_t buffer_count = 4;

class direct_sound_sink_base : boost::noncopyable
{
private:
    struct raw_buffer
    {
        raw_buffer() : ptr_(0), size_(0), written_(0)
        {
        }

        void* ptr_;
        ::DWORD size_;
        ::DWORD written_;

        std::streamsize write(const char* s, std::streamsize n)
        {
            if (n <= 0)
                return 0;

            std::size_t amt = std::min<std::size_t>(n, size_-written_);
            if (amt != 0)
            {
                std::memcpy(static_cast<char*>(ptr_)+written_, s, amt);
                written_ += amt;
            }
            return amt;
        }

        void fill(char c)
        {
            std::size_t amt = size_-written_;
            if (amt != 0)
            {
                std::memset(static_cast<char*>(ptr_)+written_, c, amt);
                written_ += amt;
            }
        }
    };

    class scoped_lock : boost::noncopyable
    {
    public:
        scoped_lock(::IDirectSoundBuffer* ptr, ::DWORD offset, ::DWORD size)
            : ptr_(ptr)
        {
            ::HRESULT res = lock(offset, size);
            if (res == DSERR_BUFFERLOST)
            {
                direct_sound_error::check(ptr_->Restore());
                direct_sound_error::check(lock(offset, size));
            }
            else
                direct_sound_error::check(res);
        }

        ~scoped_lock()
        {
            ptr_->Unlock(
                buf1_.ptr_, buf1_.written_, buf2_.ptr_, buf2_.written_);
        }

        std::streamsize write(const char* s, std::streamsize n)
        {
            std::streamsize amt = buf1_.write(s, n);
            s += amt;
            n -= amt;
            amt += buf2_.write(s, n);
            return amt;
        }

        void fill(char c)
        {
            buf1_.fill(c);
            buf2_.fill(c);
        }

    private:
        ::IDirectSoundBuffer* ptr_;
        raw_buffer buf1_;
        raw_buffer buf2_;

        ::HRESULT lock(::DWORD offset, ::DWORD size)
        {
            return ptr_->Lock(offset, size,
                &buf1_.ptr_, &buf1_.size_,
                &buf2_.ptr_, &buf2_.size_, 0);
        }
    };

public:
    explicit direct_sound_sink_base(
        const boost::shared_ptr< ::IDirectSoundBuffer>& ptr) : ptr_(ptr)
    {
    }

    void set_notification(const ::DSBPOSITIONNOTIFY* data, ::DWORD size)
    {
        detail::direct_sound_notify notify(ptr_.get());
        notify.set(data, size);
    }

    std::streamsize write(::DWORD offset, const char* s, std::streamsize n)
    {
        try
        {
            scoped_lock lock(ptr_.get(), offset, n);
            return lock.write(s, n);
        }
        catch (const direct_sound_error& e)
        {
            if (e.error() == DSERR_BUFFERLOST)
                return 0;
            else
                throw;
        }
    }

    void fill(::DWORD offset, char c, std::streamsize n)
    {
        scoped_lock lock(ptr_.get(), offset, n);
        lock.fill(c);
    }

    void play(::DWORD priority)
    {
        ptr_->Play(0, priority, DSBPLAY_LOOPING);
    }

    void stop()
    {
        ptr_->Stop();
    }

private:
    boost::shared_ptr< ::IDirectSoundBuffer> ptr_;
};

} // namespace

class direct_sound_sink::impl : private direct_sound_sink_base
{
    typedef direct_sound_sink_base base_type;

public:
    impl(const boost::shared_ptr< ::IDirectSoundBuffer>& ptr,
            const pcm_format& f, ::DWORD buffer_size)
        : base_type(ptr), offset_(0), buffer_size_(buffer_size)
        , is_open_(true), format_(f)
    {
        ::DSBPOSITIONNOTIFY pos[buffer_count];
        for (std::size_t i = 0; i < buffer_count; ++i)
        {
            pos[i].dwOffset = buffer_size*(i+1) - 1;
            pos[i].hEventNotify = events_[i].get();
        }
        set_notification(pos, buffer_count);
    }

    ~impl()
    {
        try
        {
            if (is_open_)
                stop();
        }
        catch (...)
        {
        }
    }

    pcm_format format() const
    {
        return format_;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n > 0)
        {
            if (offset_ % buffer_size_ == 0)
            {
                std::size_t index = offset_ / buffer_size_;
                events_[index].wait();
            }

            std::streamsize size =
                std::min<std::streamsize>(n,
                    buffer_size_ - offset_%buffer_size_);

            std::streamsize amt = base_type::write(offset_, s, size);

            // if the buffer is lost, the request is ignored
            if (amt == 0)
                amt = size;

            s += amt;
            n -= amt;
            total += amt;
            offset_ += amt;
            offset_ %= (buffer_size_*buffer_count);

            if (offset_ % buffer_size_ == 0)
                play(0);
        }
        return total;
    }

    void close()
    {
        if (!is_open_)
            return;
        is_open_ = false;

        try
        {
            if (offset_ % buffer_size_ != 0)
            {
                std::streamsize amt = buffer_size_ - offset_%buffer_size_;
                base_type::fill(offset_, 0, amt);
                play(0);
                offset_ += amt;
                offset_ %= (buffer_size_*buffer_count);
            }
            std::size_t index = offset_ / buffer_size_;
            events_[index].wait();
            base_type::fill(offset_, 0, buffer_size_);
            events_[(index+buffer_count-1)%buffer_count].wait();
        }
        catch (...)
        {
            stop();
            throw;
        }
        stop();
    }

private:
    auto_reset_event events_[buffer_count];
    ::DWORD offset_;
    ::DWORD buffer_size_;
    bool is_open_;
    pcm_format format_;
};

class direct_sound_device::impl : boost::noncopyable
{
public:
    impl()
    {
        direct_sound_error::check(::DirectSoundCreate(0, &ptr_, 0));
    }

    explicit impl(const uuid& driver_guid)
    {
        if (driver_guid.is_null())
        {
            direct_sound_error::check(::DirectSoundCreate(0, &ptr_, 0));
            return;
        }

        ::GUID guid;
        driver_guid.copy(guid);
        direct_sound_error::check(::DirectSoundCreate(&guid, &ptr_, 0));
    }

    ~impl()
    {
        ptr_->Release();
    }

    void set_cooperative_level(::HWND hwnd, ::DWORD level)
    {
        direct_sound_error::check(ptr_->SetCooperativeLevel(hwnd, level));
    }

    void format(const pcm_format& f)
    {
        ::DSBUFFERDESC desc;
        std::memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc); 
        desc.dwFlags = DSBCAPS_PRIMARYBUFFER;

        ::IDirectSoundBuffer* buf_ptr;
        direct_sound_error::check(ptr_->CreateSoundBuffer(&desc, &buf_ptr, 0));

        boost::shared_ptr<
            ::IDirectSoundBuffer> deleter(buf_ptr, com_release());

        detail::wave_format_ex fmt(f);
        direct_sound_error::check(buf_ptr->SetFormat(&fmt));
    }

    ::IDirectSoundBuffer* create_buffer(
        const pcm_format& f, std::size_t buffer_size)
    {
        detail::wave_format_ex fmt(f);

        ::DSBUFFERDESC desc;
        std::memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc); 
        desc.dwFlags =
            DSBCAPS_CTRLPOSITIONNOTIFY |
            DSBCAPS_GETCURRENTPOSITION2 |
            DSBCAPS_GLOBALFOCUS ;
        desc.dwBufferBytes = buffer_size * buffer_count;
        desc.lpwfxFormat = &fmt;

        ::IDirectSoundBuffer* buf_ptr;
        direct_sound_error::check(ptr_->CreateSoundBuffer(&desc, &buf_ptr, 0));
        return buf_ptr;
    }

private:
    ::IDirectSound* ptr_;
};

direct_sound_sink::direct_sound_sink(
    ::IDirectSoundBuffer* p, const pcm_format& f, std::size_t buffer_size)
{
    boost::shared_ptr< ::IDirectSoundBuffer> tmp(p, com_release());

    pimpl_.reset(new impl(tmp, f, buffer_size));
}

pcm_format direct_sound_sink::format() const
{
    return pimpl_->format();
}

std::streamsize direct_sound_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

void direct_sound_sink::close()
{
    pimpl_->close();
}

direct_sound_device::direct_sound_device()
    : pimpl_(new impl)
{
}

direct_sound_device::direct_sound_device(const uuid& driver_guid)
    : pimpl_(new impl(driver_guid))
{
}

void direct_sound_device::set_cooperative_level(void* hwnd, ::DWORD level)
{
    if (hwnd == 0)
    {
        hwnd = ::GetForegroundWindow();
        if (hwnd == 0)
            hwnd = ::GetDesktopWindow();
    }

    pimpl_->set_cooperative_level(static_cast< ::HWND>(hwnd), level);
}

void direct_sound_device::format(const pcm_format& f)
{
    pimpl_->format(f);
}

direct_sound_sink direct_sound_device::create_buffer(
    const pcm_format& f, std::size_t buffer_size)
{
    ::IDirectSoundBuffer* tmp = pimpl_->create_buffer(f, buffer_size);
    return direct_sound_sink(tmp, f, buffer_size);
}

direct_sound_sink direct_sound_device::create_buffer(const pcm_format& f)
{
    const std::size_t buffer_size = f.optimal_buffer_size();
    ::IDirectSoundBuffer* tmp = pimpl_->create_buffer(f, buffer_size);
    return direct_sound_sink(tmp, f, buffer_size);
}

} } // End namespaces audio, hamigaki.
