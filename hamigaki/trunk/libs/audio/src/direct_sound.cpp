//  direct_sound.cpp: DirectSound device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#if defined(HAMIGAKI_HAS_DXSDK)
#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/direct_sound.hpp>
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <cctype>
#include <stdexcept>

#include <hamigaki/detail/windows/auto_reset_event.hpp>
#include <hamigaki/detail/windows/com_release.hpp>
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

struct wave_format_ex : public ::WAVEFORMATEX
{
    typedef ::WAVEFORMATEX type;

    explicit wave_format_ex(const pcm_format& f)
    {
        BOOST_ASSERT(f.channels > 0);
        BOOST_ASSERT(f.channels <= 0xFFFF);
        BOOST_ASSERT(f.block_size() > 0);
        BOOST_ASSERT(f.block_size() <= 0xFFFF);
        BOOST_ASSERT(f.bits() > 0);
        BOOST_ASSERT(f.bits() <= 0xFFFF);

        std::memset(static_cast<type*>(this), 0, sizeof(type));
        wFormatTag = WAVE_FORMAT_PCM;
        nChannels = static_cast<unsigned short>(f.channels);
        nSamplesPerSec = f.rate;
        nBlockAlign = static_cast<unsigned short>(f.block_size());
        nAvgBytesPerSec = f.rate * f.block_size();
        wBitsPerSample = static_cast<unsigned short>(f.bits());
        cbSize = 0;
    }
};

class direct_sound_notify : boost::noncopyable
{
public:
    template<class Interface>
    explicit direct_sound_notify(Interface* buf_ptr)
    {
        // IID_IDirectSoundNotify
        const ::GUID iid =
        {
            0xB0210783, 0x89CD, 0x11D0,
            { 0xAF, 0x08, 0x00, 0xA0, 0xC9, 0x25, 0xCD, 0x16 }
        };

        void* tmp;
        direct_sound_error::check(buf_ptr->QueryInterface(iid, &tmp));
        ptr_ = static_cast< ::IDirectSoundNotify*>(tmp);
    }

    ~direct_sound_notify()
    {
        ptr_->Release();
    }

    void set(const ::DSBPOSITIONNOTIFY* data, ::DWORD size)
    {
        direct_sound_error::check(
            ptr_->SetNotificationPositions(size, data));
    }

private:
    ::IDirectSoundNotify* ptr_;
};

class direct_sound_buffer_base : boost::noncopyable
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
    explicit direct_sound_buffer_base(
        const boost::shared_ptr< ::IDirectSoundBuffer>& ptr) : ptr_(ptr)
    {
    }

    void set_notification(const ::DSBPOSITIONNOTIFY* data, ::DWORD size)
    {
        direct_sound_notify notify(ptr_.get());
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

class direct_sound_capture_buffer_base : boost::noncopyable
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
    explicit direct_sound_capture_buffer_base(
        const boost::shared_ptr< ::IDirectSoundCaptureBuffer>& ptr) : ptr_(ptr)
    {
    }

    void set_notification(const ::DSBPOSITIONNOTIFY* data, ::DWORD size)
    {
        direct_sound_notify notify(ptr_.get());
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

::BOOL CALLBACK ds_enum_callback_func(
    ::GUID* lpGuid, const char* lpcstrDescription,
    const char* lpcstrModule, void* lpContext)
{
    try
    {
        char guid[64];
        if (lpGuid)
        {
            ::wsprintfA(guid,
                "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                lpGuid->Data1, lpGuid->Data2, lpGuid->Data3,
                lpGuid->Data4[0], lpGuid->Data4[1],
                lpGuid->Data4[2], lpGuid->Data4[3],
                lpGuid->Data4[4], lpGuid->Data4[5],
                lpGuid->Data4[6], lpGuid->Data4[7]);
        }
        else
            guid[0] = '\0';

        direct_sound::device_info info;
        info.driver_guid = guid;
        info.description = lpcstrDescription;
        info.module_name = lpcstrModule;

        detail::ds_enum_callback_base* ptr =
            static_cast<detail::ds_enum_callback_base*>(lpContext);
        return ptr->next(info) ? TRUE : FALSE;
    }
    catch (...)
    {
    }
    return FALSE;
}

unsigned get_guid_digit_aux(const std::string& s, std::string::size_type& i)
{
    using namespace std;
    while (i < s.size())
    {
        const char c = s[i++];
        if (isdigit(c))
            return c - '0';
        // support non-ASCII encoding
        else if ((c == 'A') || (c == 'a'))
            return 10;
        else if ((c == 'B') || (c == 'b'))
            return 11;
        else if ((c == 'C') || (c == 'c'))
            return 12;
        else if ((c == 'D') || (c == 'd'))
            return 13;
        else if ((c == 'E') || (c == 'e'))
            return 14;
        else if ((c == 'F') || (c == 'f'))
            return 15;
    }
    throw std::invalid_argument("invalid GUID");
    BOOST_UNREACHABLE_RETURN(0)
}

unsigned get_guid_digit(const std::string& s, std::string::size_type& i)
{
    unsigned n1 = get_guid_digit_aux(s, i);
    unsigned n2 = get_guid_digit_aux(s, i);
    return (n1 << 4) | n2;
}

#if defined(HAMIGAKI_AUDIO_NO_DS_ENUM) || \
    defined(HAMIGAKI_AUDIO_NO_CREATE_DS_CAPTRUE)
class dynamic_link_library : boost::noncopyable
{
public:
    explicit dynamic_link_library(const char* name)
        : handle_(::LoadLibraryA(name))
    {
    }

    ~dynamic_link_library()
    {
        if (handle_ != 0)
            ::FreeLibrary(handle_);
    }

    ::FARPROC get_proc_address(const char* name)
    {
        if (handle_ != 0)
            return ::GetProcAddress(handle_, name);
        else
            return 0;
    }

private:
    ::HMODULE handle_;
};

typedef HRESULT (WINAPI *DirectSoundEnumerateFuncPtr)(
    LPDSENUMCALLBACK lpDSEnumCallback,
    LPVOID lpContext
);

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
    if (::FARPROC p = dsound.get_proc_address("DirectSoundCaptureCreate"))
    {
        DirectSoundCaptureCreateFuncPtr func_ptr =
            reinterpret_cast<DirectSoundCaptureCreateFuncPtr>(p);
        return (*func_ptr)(lpcGUID, lplpDSC, pUnkOuter);
    }
    else
        throw std::runtime_error("DirectSoundCaptureCreate() unsupported");
    BOOST_UNREACHABLE_RETURN(DSERR_UNSUPPORTED)
#endif
}

} // namespace

namespace detail
{

HAMIGAKI_AUDIO_DECL
void direct_sound_enumerate_impl(ds_enum_callback_base* ptr)
{
#if !defined(HAMIGAKI_AUDIO_NO_DS_ENUM)
    ::DirectSoundEnumerateA(&ds_enum_callback_func, ptr);
#else
    dynamic_link_library dsound("dsound.dll");
    if (::FARPROC p = dsound.get_proc_address("DirectSoundEnumerateA"))
    {
        DirectSoundEnumerateFuncPtr func_ptr =
            reinterpret_cast<DirectSoundEnumerateFuncPtr>(p);
        (*func_ptr)(&ds_enum_callback_func, ptr);
    }
    else
        throw std::runtime_error("DirectSoundEnumerateA() unsupported");
#endif
}

} // namespace detail

class direct_sound_buffer::impl : private direct_sound_buffer_base
{
    typedef direct_sound_buffer_base base_type;

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

    std::streamsize write(const char* s, std::streamsize n)
    {
        std::streamsize total = 0;
        while (n != 0)
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
        return (total != 0) ? total : -1;
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
            }
            std::size_t index = offset_ / buffer_size_;
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

    explicit impl(const std::string& guid_str)
    {
        if (guid_str.empty())
        {
            direct_sound_error::check(::DirectSoundCreate(0, &ptr_, 0));
            return;
        }

        ::GUID guid;
        std::memset(&guid, 0, sizeof(guid));

        std::string::size_type pos = 0;
        unsigned n1 = get_guid_digit(guid_str, pos);
        unsigned n2 = get_guid_digit(guid_str, pos);
        unsigned n3 = get_guid_digit(guid_str, pos);
        unsigned n4 = get_guid_digit(guid_str, pos);
        guid.Data1 = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;

        n1 = get_guid_digit(guid_str, pos);
        n2 = get_guid_digit(guid_str, pos);
        guid.Data2 = static_cast<unsigned short>((n1 << 8) | n2);

        n1 = get_guid_digit(guid_str, pos);
        n2 = get_guid_digit(guid_str, pos);
        guid.Data3 = static_cast<unsigned short>((n1 << 8) | n2);

        for (std::size_t i = 0; i < 8; ++i)
        {
            guid.Data4[i] =
                static_cast<unsigned char>(get_guid_digit(guid_str, pos));
        }

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

        wave_format_ex fmt(f);
        direct_sound_error::check(buf_ptr->SetFormat(&fmt));
    }

    ::IDirectSoundBuffer* create_buffer(
        const pcm_format& f, std::size_t buffer_size)
    {
        wave_format_ex fmt(f);

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

direct_sound_buffer::direct_sound_buffer(
    ::IDirectSoundBuffer* p, const pcm_format& f, std::size_t buffer_size)
{
    boost::shared_ptr< ::IDirectSoundBuffer> tmp(p, com_release());

    pimpl_.reset(new impl(tmp, f, buffer_size));
}

pcm_format direct_sound_buffer::format() const
{
    return pimpl_->format();
}

std::streamsize direct_sound_buffer::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

void direct_sound_buffer::close()
{
    pimpl_->close();
}

direct_sound_device::direct_sound_device()
    : pimpl_(new impl)
{
}

direct_sound_device::direct_sound_device(const std::string& guid)
    : pimpl_(new impl(guid))
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

direct_sound_buffer direct_sound_device::create_buffer(
    const pcm_format& f, std::size_t buffer_size)
{
    ::IDirectSoundBuffer* tmp = pimpl_->create_buffer(f, buffer_size);
    return direct_sound_buffer(tmp, f, buffer_size);
}

direct_sound_buffer direct_sound_device::create_buffer(const pcm_format& f)
{
    const std::size_t buffer_size = f.optimal_buffer_size();
    ::IDirectSoundBuffer* tmp = pimpl_->create_buffer(f, buffer_size);
    return direct_sound_buffer(tmp, f, buffer_size);
}

class direct_sound_capture_buffer::impl
    : private direct_sound_capture_buffer_base
{
    typedef direct_sound_capture_buffer_base base_type;

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
        while (n != 0)
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

    explicit impl(const std::string& guid_str)
    {
        if (guid_str.empty())
        {
            direct_sound_error::check(
                direct_sound_capture_create(0, &ptr_, 0));
            return;
        }

        ::GUID guid;
        std::memset(&guid, 0, sizeof(guid));

        std::string::size_type pos = 0;
        unsigned n1 = get_guid_digit(guid_str, pos);
        unsigned n2 = get_guid_digit(guid_str, pos);
        unsigned n3 = get_guid_digit(guid_str, pos);
        unsigned n4 = get_guid_digit(guid_str, pos);
        guid.Data1 = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;

        n1 = get_guid_digit(guid_str, pos);
        n2 = get_guid_digit(guid_str, pos);
        guid.Data2 = static_cast<unsigned short>((n1 << 8) | n2);

        n1 = get_guid_digit(guid_str, pos);
        n2 = get_guid_digit(guid_str, pos);
        guid.Data3 = static_cast<unsigned short>((n1 << 8) | n2);

        for (std::size_t i = 0; i < 8; ++i)
        {
            guid.Data4[i] =
                static_cast<unsigned char>(get_guid_digit(guid_str, pos));
        }

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
        wave_format_ex fmt(f);

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

direct_sound_capture_buffer::direct_sound_capture_buffer(
    ::IDirectSoundCaptureBuffer* p,
    const pcm_format& f, std::size_t buffer_size)
{
    boost::shared_ptr<
        ::IDirectSoundCaptureBuffer> tmp(p, com_release());

    pimpl_.reset(new impl(tmp, f, buffer_size));
}

pcm_format direct_sound_capture_buffer::format() const
{
    return pimpl_->format();
}

std::streamsize direct_sound_capture_buffer::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void direct_sound_capture_buffer::close()
{
    pimpl_->close();
}

direct_sound_capture::direct_sound_capture()
    : pimpl_(new impl)
{
}

direct_sound_capture::direct_sound_capture(const std::string& guid)
    : pimpl_(new impl(guid))
{
}

direct_sound_capture_buffer direct_sound_capture::create_buffer(
    const pcm_format& f, std::size_t buffer_size)
{
    ::IDirectSoundCaptureBuffer* tmp = pimpl_->create_buffer(f, buffer_size);
    return direct_sound_capture_buffer(tmp, f, buffer_size);
}

direct_sound_capture_buffer
direct_sound_capture::create_buffer(const pcm_format& f)
{
    const std::size_t buffer_size = f.optimal_buffer_size();
    ::IDirectSoundCaptureBuffer* tmp = pimpl_->create_buffer(f, buffer_size);
    return direct_sound_capture_buffer(tmp, f, buffer_size);
}

} } // End namespaces audio, hamigaki.

#endif // defined(HAMIGAKI_HAS_DXSDK)
