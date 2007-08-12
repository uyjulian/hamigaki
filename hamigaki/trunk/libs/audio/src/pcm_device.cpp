// pcm_device.cpp: a simple pcm device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/pcm_device.hpp>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/noncopyable.hpp>
#include <stdexcept>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/iostreams/arbitrary_positional_facade.hpp>
    #include <boost/ptr_container/ptr_vector.hpp>
    #include <vector>
    #include "detail/wave_format_ex.hpp"
    #include <windows.h>
    #include <mmsystem.h>
#elif defined(__APPLE__)
    #include <hamigaki/integer/auto_min.hpp>
    #include <boost/scoped_array.hpp>
    #include <AudioUnit/AudioUnit.h>
    #include <CoreServices/CoreServices.h>
    #include <pthread.h>
#else
    #include <sys/ioctl.h>
    #include <sys/soundcard.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

namespace hamigaki { namespace audio {

#if defined(BOOST_WINDOWS)
namespace
{

const std::size_t wave_buffer_count = 4;

class semaphore : boost::noncopyable
{
public:
    semaphore(long init_value, long max_value) :
        handle_(::CreateSemaphore(0, init_value, max_value, 0))
    {
    }

    ~semaphore()
    {
        ::CloseHandle(handle_);
    }

    long release(long n)
    {
        long prev;
        ::ReleaseSemaphore(handle_, n, &prev);
        return prev;
    }

    void wait()
    {
        ::WaitForSingleObject(handle_, INFINITE);
    }

private:
    ::HANDLE handle_;
};

class wave_buffer : boost::noncopyable
{
public:
    wave_buffer(::HWAVEOUT handle, std::size_t size) :
        handle_(handle), buffer_(size), pos_(0)
    {
        std::memset(&header_, 0, sizeof(header_));

        header_.lpData = &buffer_[0];
        header_.dwBufferLength  = size;
        header_.dwFlags = 0;

        ::MMRESULT res =
            ::waveOutPrepareHeader(handle_, &header_, sizeof(header_));
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("cannot prepare wave buffer");
    }

    ~wave_buffer()
    {
        ::waveOutUnprepareHeader(handle_, &header_, sizeof(header_));
    }

    void flush()
    {
        if (pos_ != 0)
        {
            header_.dwBufferLength = pos_;
            pos_ = 0;
            ::waveOutWrite(handle_, &header_, sizeof(header_));
        }
    }

    bool write(const char* data, std::size_t size, std::size_t& written)
    {
        written = (std::min)(buffer_.size() - pos_, size);
        std::memcpy(&buffer_[pos_], data, written);
        pos_ += written;
        if (pos_ == buffer_.size())
        {
            pos_ = 0;
            header_.dwBufferLength = buffer_.size();
            ::MMRESULT res = ::waveOutWrite(handle_, &header_, sizeof(header_));
            if (res != MMSYSERR_NOERROR)
                throw BOOST_IOSTREAMS_FAILURE("wave device write error");
            return true;
        }
        return false;
    }

private:
    ::HWAVEOUT handle_;
    std::vector<char> buffer_;
    ::WAVEHDR header_;
    std::size_t pos_;
};

class wave_in_buffer : boost::noncopyable
{
public:
    wave_in_buffer(::HWAVEIN handle, std::size_t size) :
        handle_(handle), buffer_(size), pos_(0)
    {
        std::memset(&header_, 0, sizeof(header_));

        header_.lpData = &buffer_[0];
        header_.dwBufferLength = size;
        header_.dwFlags = 0;

        ::MMRESULT res =
            ::waveInPrepareHeader(handle_, &header_, sizeof(header_));
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("cannot prepare wave buffer");
    }

    ~wave_in_buffer()
    {
        ::waveInUnprepareHeader(handle_, &header_, sizeof(header_));
    }

    void add()
    {
        pos_ = 0;
#if 0
        ::waveInUnprepareHeader(handle_, &header_, sizeof(header_));
        std::memset(&header_, 0, sizeof(header_));
        header_.lpData = &buffer_[0];
        header_.dwBufferLength = buffer_.size();
        header_.dwFlags = 0;
        ::waveInPrepareHeader(handle_, &header_, sizeof(header_));
#else
        // Is this OK?
        header_.dwBytesRecorded = 0;
        header_.dwFlags &= ~WHDR_DONE;
#endif

        ::MMRESULT res = ::waveInAddBuffer(handle_, &header_, sizeof(header_));
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("waveInAddBuffer failed");
    }

    bool read(char* data, std::size_t size, std::size_t& amt)
    {
        amt = (std::min)(buffer_.size() - pos_, size);
        std::memcpy(data, &buffer_[pos_], amt);
        pos_ += amt;
        if (pos_ == buffer_.size())
            return true;
        return false;
    }

private:
    ::HWAVEIN handle_;
    std::vector<char> buffer_;
    ::WAVEHDR header_;
    std::size_t pos_;
};

} // namespace

class pcm_sink::impl
    : public hamigaki::iostreams::
        arbitrary_positional_facade<
#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
            pcm_sink::
#endif
            impl, char, 4
        >
    , private boost::noncopyable
{
    friend class hamigaki::iostreams::core_access;

public:
    impl(const pcm_format& f, std::size_t buffer_size)
        : impl::arbitrary_positional_facade_(f.block_size())
        , sema_(wave_buffer_count, wave_buffer_count)
        , block_size_(f.block_size())
        , pos_(0), has_buffer_(false), format_(f)
    {
        if (buffer_size % block_size_ != 0)
            throw BOOST_IOSTREAMS_FAILURE("invalid buffer size");

        detail::wave_format_ex fmt(f);

        ::MMRESULT res = ::waveOutOpen(
            &handle_, WAVE_MAPPER, &fmt,
            reinterpret_cast< ::DWORD_PTR>(&pcm_sink::impl::Callback),
            reinterpret_cast< ::DWORD_PTR>(&sema_),
            CALLBACK_FUNCTION);
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("waveOutOpen failed");

        for (std::size_t i = 0; i < wave_buffer_count; ++i)
            buffers_.push_back(new wave_buffer(handle_, buffer_size));
    }

    ~impl()
    {
        ::waveOutReset(handle_);
        ::waveOutClose(handle_);
    }

    pcm_format format() const
    {
        return format_;
    }

    bool flush()
    {
        if (has_buffer_)
        {
            buffers_[pos_].flush();
            has_buffer_ = false;
            pos_ = (pos_ + 1) % buffers_.size();
        }

        for (std::size_t i = 0; i < buffers_.size(); ++i)
            sema_.wait();
        sema_.release(buffers_.size());

        return true;
    }

    void close()
    {
        flush();
        ::waveOutReset(handle_);
    }

private:
    ::HWAVEOUT handle_;
    semaphore sema_;
    boost::ptr_vector<wave_buffer> buffers_;
    std::size_t block_size_;
    std::size_t pos_;
    bool has_buffer_;
    pcm_format format_;

    static void CALLBACK Callback(::HWAVEOUT hwo, ::UINT uMsg,
        ::DWORD_PTR dwInstance, ::DWORD dwParam1, ::DWORD dwParam2)
    {
        if (uMsg != WOM_DONE)
            return;

        semaphore& sema = *reinterpret_cast<semaphore*>(dwInstance);
        sema.release(1);
    }

    std::streamsize write_blocks(const char* s, std::streamsize n)
    {
        std::streamsize result = n * block_size_;
        while (n > 0)
        {
            if (!has_buffer_)
            {
                sema_.wait();
                has_buffer_ = true;
            }

            std::size_t written;
            if (buffers_[pos_].write(s, n*block_size_, written))
            {
                has_buffer_ = false;
                pos_ = (pos_ + 1) % buffers_.size();
                n -= written/block_size_;
                s += written;
            }
            else
                break;
        }
        return result;
    }
};

class pcm_source::impl
    : public hamigaki::iostreams::
        arbitrary_positional_facade<
#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
            pcm_source::
#endif
            impl, char, 4
        >
    , private boost::noncopyable
{
    friend class hamigaki::iostreams::core_access;

public:
    impl(const pcm_format& f, std::size_t buffer_size)
        : impl::arbitrary_positional_facade_(f.block_size())
        , sema_(0, wave_buffer_count)
        , block_size_(f.block_size())
        , pos_(0), has_buffer_(false), format_(f)
    {
        if (buffer_size % block_size_ != 0)
            throw BOOST_IOSTREAMS_FAILURE("invalid buffer size");

        detail::wave_format_ex fmt(f);

        ::MMRESULT res = ::waveInOpen(
            &handle_, WAVE_MAPPER, &fmt,
            reinterpret_cast< ::DWORD_PTR>(&pcm_source::impl::Callback),
            reinterpret_cast< ::DWORD_PTR>(&sema_),
            CALLBACK_FUNCTION);
        if (res != MMSYSERR_NOERROR)
            throw BOOST_IOSTREAMS_FAILURE("waveInOpen failed");

        for (std::size_t i = 0; i < wave_buffer_count; ++i)
        {
            buffers_.push_back(new wave_in_buffer(handle_, buffer_size));
            buffers_[i].add();
        }
    }

    ~impl()
    {
        close();
        ::waveInReset(handle_);
        ::waveInClose(handle_);
    }

    pcm_format format() const
    {
        return format_;
    }

    void close()
    {
        ::waveInStop(handle_);
        ::waveInReset(handle_);

        std::size_t count = buffers_.size();
        if (has_buffer_)
            --count;
        for (std::size_t i = 0; i < count; ++i)
            sema_.wait();
        has_buffer_ = false;

        for (std::size_t i = 0; i < wave_buffer_count; ++i)
            buffers_[i].add();
    }

private:
    ::HWAVEIN handle_;
    semaphore sema_;
    boost::ptr_vector<wave_in_buffer> buffers_;
    std::size_t block_size_;
    std::size_t pos_;
    bool has_buffer_;
    pcm_format format_;

    static void CALLBACK Callback(::HWAVEOUT hwo, ::UINT uMsg,
        ::DWORD_PTR dwInstance, ::DWORD dwParam1, ::DWORD dwParam2)
    {
        if (uMsg != WIM_DATA)
            return;

        semaphore& sema = *reinterpret_cast<semaphore*>(dwInstance);
        sema.release(1);
    }

    std::streamsize read_blocks(char* s, std::streamsize n)
    {
        ::waveInStart(handle_);
        std::streamsize total = 0;
        while (n > 0)
        {
            if (!has_buffer_)
            {
                sema_.wait();
                has_buffer_ = true;
            }

            std::size_t amt;
            if (buffers_[pos_].read(s, n*block_size_, amt))
            {
                has_buffer_ = false;
                buffers_[pos_].add();
                pos_ = (pos_ + 1) % buffers_.size();
                n -= amt/block_size_;
                s += amt;
                total += amt;
            }
            else
                break;
        }
        return (total != 0) ? total : -1;
    }
};

#elif defined(__APPLE__) // not defined(BOOST_WINDOWS) and defined(__APPLE__)
namespace
{

const std::size_t buffer_count = 4;

::Component find_default_audio_output()
{
    ::ComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

    ::Component c = ::FindNextComponent(0, &desc);
    if (c == 0)
        throw BOOST_IOSTREAMS_FAILURE("audio device not found");
    return c;
}

inline bool is_float(sample_format_type type)
{
    const std::streamsize table[] =
    {
        false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        true,  true,  true,  true
    };
    BOOST_STATIC_ASSERT(sizeof(table)/sizeof(table[0]) == 20);
    BOOST_ASSERT(static_cast<int>(type) < 20);

    return table[type];
}

inline bool is_big_endian(sample_format_type type)
{
    const std::streamsize table[] =
    {
        false, false, false, true,  false, true,  false, true,
        false, true,  false, true,  false, true,  false, true,
        false, true,  false, true
    };
    BOOST_STATIC_ASSERT(sizeof(table)/sizeof(table[0]) == 20);
    BOOST_ASSERT(static_cast<int>(type) < 20);

    return table[type];
}

inline bool is_signed_int(sample_format_type type)
{
    const std::streamsize table[] =
    {
        false, true,  true,  true,  true,  true,  true,  true,
        true,  true,  true,  true,  true,  true,  true,  true,
        false, false, false, false 
    };
    BOOST_STATIC_ASSERT(sizeof(table)/sizeof(table[0]) == 20);
    BOOST_ASSERT(static_cast<int>(type) < 20);

    return table[type];
}

inline bool is_packed(sample_format_type type)
{
    const std::streamsize table[] =
    {
        true,  true,  true,  true,  true,  true,  true,  true,
        false, false, false, false, false, false, false, false,
        true,  true,  true,  true
    };
    BOOST_STATIC_ASSERT(sizeof(table)/sizeof(table[0]) == 20);
    BOOST_ASSERT(static_cast<int>(type) < 20);

    return table[type];
}

::AudioStreamBasicDescription make_au_format(const pcm_format& f)
{
    ::AudioStreamBasicDescription fmt;
    fmt.mSampleRate = f.rate;
    fmt.mFormatID = kAudioFormatLinearPCM;

    fmt.mFormatFlags = 0;
    if (is_float(f.type))
        fmt.mFormatFlags |= kLinearPCMFormatFlagIsFloat;
    if (is_big_endian(f.type))
        fmt.mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;
    if (is_signed_int(f.type))
        fmt.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
    if (is_packed(f.type))
        fmt.mFormatFlags |= kLinearPCMFormatFlagIsPacked;
    if (fmt.mFormatFlags == 0)
        fmt.mFormatFlags = kLinearPCMFormatFlagsAreAllClear;

    fmt.mBytesPerPacket = f.block_size();
    fmt.mFramesPerPacket = 1;
    fmt.mBytesPerFrame = fmt.mBytesPerPacket / fmt.mFramesPerPacket;
    fmt.mChannelsPerFrame = f.channels;
    fmt.mBitsPerChannel = f.bits();
    fmt.mReserved = 0;

    return fmt;
}

::UInt32 get_device_buffer_size(::AudioDeviceID dev)
{
    ::UInt32 data;
    ::UInt32 size = sizeof(data);

    ::OSStatus err = ::AudioDeviceGetProperty(
        dev, 0, 0, kAudioDevicePropertyBufferSize, &size, &data);

    if (err != 0)
    {
        throw BOOST_IOSTREAMS_FAILURE(
            "cannot get the buffer size of the device");
    }

    return data;
}

class audio_unit : private boost::noncopyable
{
public:
    explicit audio_unit(::Component c)
    {
        ::OSStatus err = ::OpenAComponent(c, &handle_);
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("cannot open AudioUnit");
    }

    ~audio_unit()
    {
        ::CloseComponent(handle_);
    }

    void initialize()
    {
        ::OSStatus err = ::AudioUnitInitialize(handle_);
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("cannot initialize AudioUnit");
    }

    void uninitialize()
    {
        ::AudioUnitUninitialize(handle_);
    }

    void start()
    {
        ::OSStatus err = ::AudioOutputUnitStart(handle_);
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("cannot start AudioUnit");
    }

    void stop()
    {
        ::AudioOutputUnitStop(handle_);
    }

    ::AudioDeviceID device_id() const
    {
        return get_proprty< ::AudioDeviceID>(
            kAudioOutputUnitProperty_CurrentDevice);
    }

    void input_format(const ::AudioStreamBasicDescription& fmt)
    {
        set_proprty(
            kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, fmt);
    }

    void set_render_callback(::AURenderCallback proc, void* ctx)
    {
        ::AURenderCallbackStruct data;
        data.inputProc = proc;
        data.inputProcRefCon = ctx;

        set_proprty(
            kAudioUnitProperty_SetRenderCallback,
            kAudioUnitScope_Input, 0, data);
    }

private:
    ::AudioUnit handle_;

    template<class T>
    T get_proprty(
        ::AudioUnitPropertyID id, ::AudioUnitScope scope,
        ::AudioUnitElement elem) const
    {
        T data;
        ::UInt32 size = sizeof(T);

        ::OSStatus err =
            ::AudioUnitGetProperty(handle_, id, scope, elem, &data, &size);
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("failed AudioUnitGetProperty()");

        return data;
    }

    template<class T>
    T get_proprty(::AudioUnitPropertyID id) const
    {
        return get_proprty<T>(id, kAudioUnitScope_Global, 0);
    }

    template<class T>
    void set_proprty(
        ::AudioUnitPropertyID id, ::AudioUnitScope scope,
        ::AudioUnitElement elem, const T& data) const
    {
        ::OSStatus err =
            ::AudioUnitSetProperty(handle_, id, scope, elem, &data, sizeof(T));
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("failed AudioUnitSetProperty()");
    }

    template<class T>
    void set_proprty(::AudioUnitPropertyID id, const T& data) const
    {
        set_proprty(id, kAudioUnitScope_Global, 0, data);
    }
};

class condition;
class mutex : private boost::noncopyable
{
    friend class condition;

public:
    class scoped_lock
    {
    public:
        explicit scoped_lock(mutex& m) : mutex_(m)
        {
            mutex_.lock();
        }

        ~scoped_lock()
        {
            mutex_.unlock();
        }

    private:
        mutex& mutex_;
    };

    mutex()
    {
        ::pthread_mutex_init(&handle_, 0);
    }

    ~mutex()
    {
        ::pthread_mutex_destroy(&handle_);
    }

    void lock()
    {
        ::pthread_mutex_lock(&handle_);
    }

    void unlock()
    {
        ::pthread_mutex_unlock(&handle_);
    }

private:
    ::pthread_mutex_t handle_;
};

class condition : private boost::noncopyable
{
public:
    condition()
    {
        ::pthread_cond_init(&handle_, 0);
    }

    ~condition()
    {
        ::pthread_cond_destroy(&handle_);
    }

    void notify_one()
    {
        ::pthread_cond_signal(&handle_);
    }

    void wait(mutex& m)
    {
        ::pthread_cond_wait(&handle_, &m.handle_);
    }

private:
    ::pthread_cond_t handle_;
};

class buffer_list
{
public:
    explicit buffer_list(std::size_t buffer_size)
        : buffer_size_(buffer_size)
        , read_index_(0)
        , write_index_(0), write_offset_(0)
        , buffer_(new char[buffer_size_*buffer_count])
    {
        for (std::size_t i = 0, off = 0; i < buffer_count; ++i)
        {
            buffers_[i] = &buffer_[off];
            off += buffer_size_;
        }
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        mutex::scoped_lock lock(mutex_);
        if (write_offset_ == 0)
        {
            while (filled_[write_index_])
                cond_.wait(mutex_);
        }

        std::streamsize amt = hamigaki::auto_min(buffer_size_-write_offset_, n);
        std::memcpy(buffers_[read_index_] + write_offset_, s, amt);

        write_offset_ += amt;
        if (write_offset_ == buffer_size_)
        {
            write_offset_ = 0;
            write_index_ = (write_index_ + 1) % buffer_count;
        }

        return amt;
    }

    void flush()
    {
        mutex::scoped_lock lock(mutex_);
        if (write_offset_ == 0)
            return;

        std::size_t rest = buffer_size_-write_offset_;
        std::memset(buffers_[read_index_] + write_offset_, 0, rest);

        write_offset_ = 0;
        write_index_ = (write_index_ + 1) % buffer_count;
    }

    void read(char* s)
    {
        mutex::scoped_lock lock(mutex_);
        if (filled_[read_index_])
        {
            std::memcpy(s, buffers_[read_index_], buffer_size_);
            filled_[read_index_] = false;
            cond_.notify_one();
            read_index_ = (read_index_ + 1) % buffer_count;
        }
        else
            std::memset(s, 0, buffer_size_);
    }

private:
    mutex mutex_;
    condition cond_;
    std::size_t buffer_size_;
    std::size_t read_index_;
    std::size_t write_index_;
    std::size_t write_offset_;
    bool filled_[buffer_count];
    boost::scoped_array<char> buffer_;
    char* buffers_[buffer_count];
};

} // namespace

class pcm_sink::impl
{
public:
    impl(const pcm_format& f, std::size_t /*buffer_size*/)
        : output_((find_default_audio_output()))
        , format_(f)
        , buffer_size_(get_device_buffer_size(output_.device_id()))
        , queue_(buffer_size_)
    {
        output_.set_render_callback(&impl::render_callback, this);
        output_.initialize();
        try
        {
            output_.input_format(make_au_format(f));
        }
        catch (...)
        {
            output_.uninitialize();
            throw;
        }
    }

    ~impl()
    {
        if (running_)
            output_.stop();
        output_.uninitialize();
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        if (!running_)
        {
            output_.start();
            running_ = true;
        }

        std::streamsize total = 0;
        while (total != n)
        {
            std::streamsize amt = queue_.write(s+total, n-total);
            total += amt;
        }
        return total;
    }

    void close()
    {
        output_.stop();
        running_ = false;
    }

    bool flush()
    {
        queue_.flush();
        if (!running_)
        {
            output_.start();
            running_ = true;
        }
        return true;
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    audio_unit output_;
    pcm_format format_;
    std::size_t buffer_size_;
    buffer_list queue_;
    bool running_;

    ::OSStatus render_callback_impl(
        ::AudioUnitRenderActionFlags* ioActionFlags,
        const ::AudioTimeStamp* inTimeStamp,
        ::UInt32 inBusNumber,
        ::UInt32 inNumberFrames,
        ::AudioBufferList* ioData)
    {
        BOOST_ASSERT(inNumberFrames == buffer_size_/format_.block_size());
        queue_.read(static_cast<char*>(ioData->mBuffers[0].mData));
        return noErr;
    }

    static ::OSStatus render_callback(
        void* inRefCon,
        ::AudioUnitRenderActionFlags* ioActionFlags,
        const ::AudioTimeStamp* inTimeStamp,
        ::UInt32 inBusNumber,
        ::UInt32 inNumberFrames,
        ::AudioBufferList* ioData)
    {
        return static_cast<impl*>(inRefCon)->render_callback_impl(
            ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    }
};

class pcm_source::impl
{
public:
    impl(const pcm_format& f, std::size_t /*buffer_size*/)
        : format_(f)
    {
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        return (n > 0) ? n : -1;
    }

    void close()
    {
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    pcm_format format_;
};

#else // not defined(BOOST_WINDOWS) and not defined(__APPLE__)
namespace
{

class dev_dsp : boost::noncopyable
{
public:
    explicit dev_dsp(int mode) :
        fd_(::open("/dev/dsp", mode))
    {
        if (fd_ == -1)
            throw BOOST_IOSTREAMS_FAILURE("cannot open /dev/dsp");
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

class pcm_sink::impl : public dev_dsp
{
public:
    impl(const pcm_format& f, std::size_t buffer_size)
        : dev_dsp(O_WRONLY), format_(f)
    {
        if (f.type == uint8)
            dev_dsp::format(AFMT_U8);
        else if (f.type == int_le16)
            dev_dsp::format(AFMT_S16_LE);
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

class pcm_source::impl : public dev_dsp
{
public:
    impl(const pcm_format& f, std::size_t buffer_size)
        : dev_dsp(O_RDONLY), format_(f)
    {
        if (f.type == uint8)
            dev_dsp::format(AFMT_U8);
        else if (f.type == int_le16)
            dev_dsp::format(AFMT_S16_LE);
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
