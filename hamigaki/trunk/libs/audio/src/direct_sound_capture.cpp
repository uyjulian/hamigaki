// direct_sound_capture.cpp: DirectSound capture device

// Copyright Takeshi Mouri 2006, 2007.
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

namespace
{

const std::size_t buffer_count = 4;

class direct_sound_source_base : boost::noncopyable
{
private:
    struct raw_buffer
    {
        raw_buffer() : ptr_(0), size_(0), read_(0)
        {
        }

        void* ptr_;
        ::DWORD size_;
        ::DWORD read_;

        std::streamsize read(char* s, std::streamsize n)
        {
            std::size_t amt = std::min<std::size_t>(n, size_-read_);
            if (amt != 0)
            {
                std::memcpy(s, static_cast<char*>(ptr_)+read_, amt);
                read_ += amt;
            }
            return amt;
        }
    };

    class scoped_lock : boost::noncopyable
    {
    public:
        scoped_lock(::IDirectSoundCaptureBuffer* ptr,
                ::DWORD offset, ::DWORD size)
            : ptr_(ptr)
        {
            direct_sound_error::check(
                ptr_->Lock(offset, size,
                    &buf1_.ptr_, &buf1_.size_,
                    &buf2_.ptr_, &buf2_.size_, 0));
        }

        ~scoped_lock()
        {
            ptr_->Unlock(
                buf1_.ptr_, buf1_.read_, buf2_.ptr_, buf2_.read_);
        }

        std::streamsize read(char* s, std::streamsize n)
        {
            std::streamsize amt = buf1_.read(s, n);
            s += amt;
            n -= amt;
            amt += buf2_.read(s, n);
            return amt;
        }

    private:
        ::IDirectSoundCaptureBuffer* ptr_;
        raw_buffer buf1_;
        raw_buffer buf2_;
    };

public:
    explicit direct_sound_source_base(
        const boost::shared_ptr< ::IDirectSoundCaptureBuffer>& ptr) : ptr_(ptr)
    {
    }

    void set_notification(const ::DSBPOSITIONNOTIFY* data, ::DWORD size)
    {
        detail::direct_sound_notify notify(ptr_.get());
        notify.set(data, size);
    }

    std::streamsize read(::DWORD offset, char* s, std::streamsize n)
    {
        scoped_lock lock(ptr_.get(), offset, n);
        return lock.read(s, n);
    }

    void start()
    {
        ptr_->Start(DSCBSTART_LOOPING);
    }

    void stop()
    {
        ptr_->Stop();
    }

private:
    boost::shared_ptr< ::IDirectSoundCaptureBuffer> ptr_;
};

#if defined(HAMIGAKI_AUDIO_NO_CREATE_DS_CAPTRUE)
typedef HRESULT (WINAPI *DirectSoundCaptureCreateFuncPtr)(
    LPGUID lpcGUID,
    LPDIRECTSOUNDCAPTURE* lplpDSC,
    LPUNKNOWN pUnkOuter
);
#endif

HRESULT direct_sound_capture_create(
    LPGUID lpcGUID,
    LPDIRECTSOUNDCAPTURE* lplpDSC,
    LPUNKNOWN pUnkOuter)
{
#if !defined(HAMIGAKI_AUDIO_NO_CREATE_DS_CAPTRUE)
    return ::DirectSoundCaptureCreate(lpcGUID, lplpDSC, pUnkOuter);
#else
    dynamic_link_library dsound("dsound.dll");
    DirectSoundCaptureCreateFuncPtr func_ptr =
        reinterpret_cast<DirectSoundCaptureCreateFuncPtr>(
            dsound.get_proc_address("DirectSoundCaptureCreate"));
    return (*func_ptr)(lpcGUID, lplpDSC, pUnkOuter);
#endif
}

} // namespace

class direct_sound_source::impl
    : private direct_sound_source_base
{
    typedef direct_sound_source_base base_type;

public:
    impl(const boost::shared_ptr< ::IDirectSoundCaptureBuffer>& ptr,
            const pcm_format& f, ::DWORD buffer_size)
        : base_type(ptr), offset_(0), buffer_size_(buffer_size)
        , is_open_(true), format_(f)
    {
        ::DSBPOSITIONNOTIFY pos[buffer_count];
        for (std::size_t i = 0; i < buffer_count; ++i)
        {
            pos[i].dwOffset = buffer_size*(i+1) - 1;
            events_[i].wait(); // reset
            pos[i].hEventNotify = events_[i].get();
        }
        set_notification(pos, buffer_count);
    }

    ~impl()
    {
        try
        {
            close();
        }
        catch (...)
        {
        }
    }

    pcm_format format() const
    {
        return format_;
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        if (offset_ % buffer_size_ == 0)
            start();
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

            std::streamsize amt = base_type::read(offset_, s, size);
            s += amt;
            n -= amt;
            total += amt;
            offset_ += amt;
            offset_ %= (buffer_size_*buffer_count);
        }
        return (total != 0) ? total : -1;
    }

    void close()
    {
        if (!is_open_)
            return;
        is_open_ = false;
        stop();
    }

private:
    auto_reset_event events_[buffer_count];
    ::DWORD offset_;
    ::DWORD buffer_size_;
    bool is_open_;
    pcm_format format_;
};

class direct_sound_capture::impl : boost::noncopyable
{
public:
    impl()
    {
        direct_sound_error::check(direct_sound_capture_create(0, &ptr_, 0));
    }

    explicit impl(const uuid& driver_guid)
    {
        if (driver_guid.is_null())
        {
            direct_sound_error::check(
                direct_sound_capture_create(0, &ptr_, 0));
            return;
        }

        ::GUID guid;
        driver_guid.copy(guid);
        direct_sound_error::check(
            direct_sound_capture_create(&guid, &ptr_, 0));
    }

    ~impl()
    {
        ptr_->Release();
    }

    ::IDirectSoundCaptureBuffer* create_buffer(
        const pcm_format& f, std::size_t buffer_size)
    {
        detail::wave_format_ex fmt(f);

        ::DSCBUFFERDESC desc;
        std::memset(&desc, 0, sizeof(desc));
        desc.dwSize = sizeof(desc); 
        desc.dwFlags = 0;
        desc.dwBufferBytes = buffer_size * buffer_count;
        desc.lpwfxFormat = &fmt;

        ::IDirectSoundCaptureBuffer* buf_ptr;
        direct_sound_error::check(
            ptr_->CreateCaptureBuffer(&desc, &buf_ptr, 0));
        return buf_ptr;
    }

private:
    ::IDirectSoundCapture* ptr_;
};

direct_sound_source::direct_sound_source(
    ::IDirectSoundCaptureBuffer* p,
    const pcm_format& f, std::size_t buffer_size)
{
    boost::shared_ptr<
        ::IDirectSoundCaptureBuffer> tmp(p, com_release());

    pimpl_.reset(new impl(tmp, f, buffer_size));
}

pcm_format direct_sound_source::format() const
{
    return pimpl_->format();
}

std::streamsize direct_sound_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void direct_sound_source::close()
{
    pimpl_->close();
}

direct_sound_capture::direct_sound_capture()
    : pimpl_(new impl)
{
}

direct_sound_capture::direct_sound_capture(const uuid& driver_guid)
    : pimpl_(new impl(driver_guid))
{
}

direct_sound_source direct_sound_capture::create_buffer(
    const pcm_format& f, std::size_t buffer_size)
{
    ::IDirectSoundCaptureBuffer* tmp = pimpl_->create_buffer(f, buffer_size);
    return direct_sound_source(tmp, f, buffer_size);
}

direct_sound_source
direct_sound_capture::create_buffer(const pcm_format& f)
{
    const std::size_t buffer_size = f.optimal_buffer_size();
    ::IDirectSoundCaptureBuffer* tmp = pimpl_->create_buffer(f, buffer_size);
    return direct_sound_source(tmp, f, buffer_size);
}

} } // End namespaces audio, hamigaki.
