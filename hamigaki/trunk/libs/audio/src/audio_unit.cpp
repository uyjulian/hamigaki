// audio_unit.cpp: Mac OS X Audio Unit device

// Copyright Takeshi Mouri 2006-2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include <hamigaki/audio/audio_unit.hpp>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/noncopyable.hpp>
#include <stdexcept>

#include <hamigaki/integer/auto_min.hpp>
#include "detail/circular_buffer.hpp"
#include <AudioToolbox/AudioConverter.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/AudioHardware.h>
#include <CoreServices/CoreServices.h>
#include <pthread.h>

namespace hamigaki { namespace audio {

namespace
{

const ::OSStatus failed_render_callback = 1000;
const ::OSStatus failed_input_callback  = 1001;
const ::OSStatus need_more_input        = 1002;

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

::Component find_auhal_output()
{
    ::ComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_HALOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;

    ::Component c = ::FindNextComponent(0, &desc);
    if (c == 0)
        throw BOOST_IOSTREAMS_FAILURE("audio device not found");
    return c;
}

::AudioDeviceID get_default_input_device()
{
    ::AudioDeviceID data;
    ::UInt32 size = sizeof(data);

    ::OSStatus err =
        ::AudioHardwareGetProperty(
            kAudioHardwarePropertyDefaultInputDevice, &size, &data);
    if (err != 0)
        throw BOOST_IOSTREAMS_FAILURE("default audio input device not found");
    return data;
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

class audio_unit_base : private boost::noncopyable
{
public:
    explicit audio_unit_base(::Component c)
    {
        ::OSStatus err = ::OpenAComponent(c, &handle_);
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("cannot open AudioUnit");
    }

    ~audio_unit_base()
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

    template<class T>
    T get_property(
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
    T get_property(::AudioUnitPropertyID id) const
    {
        return get_property<T>(id, kAudioUnitScope_Global, 0);
    }

    template<class T>
    void set_property(
        ::AudioUnitPropertyID id, ::AudioUnitScope scope,
        ::AudioUnitElement elem, const T& data) const
    {
        ::OSStatus err =
            ::AudioUnitSetProperty(handle_, id, scope, elem, &data, sizeof(T));
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("failed AudioUnitSetProperty()");
    }

    template<class T>
    void set_property(::AudioUnitPropertyID id, const T& data) const
    {
        set_property(id, kAudioUnitScope_Global, 0, data);
    }

    void render(
        ::AudioUnitRenderActionFlags* ioActionFlags,
        const ::AudioTimeStamp* inTimeStamp,
        ::UInt32 inBusNumber,
        ::UInt32 inNumberFrames,
        ::AudioBufferList* ioData)
    {
        ::OSStatus err = ::AudioUnitRender(
            handle_, ioActionFlags, inTimeStamp, inBusNumber,
            inNumberFrames, ioData
        );
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("failed AudioUnitRender()");
    }

private:
    ::AudioUnit handle_;
};

class audio_unit
{
public:
    explicit audio_unit(::Component c) : base_(c)
    {
        base_.initialize();
    }

    ~audio_unit()
    {
        base_.uninitialize();
    }

    void start()
    {
        base_.start();
    }

    void stop()
    {
        base_.stop();
    }

    void device_id(::AudioDeviceID id)
    {
        base_.set_property(kAudioOutputUnitProperty_CurrentDevice, id);
    }

    ::AudioStreamBasicDescription input_format(::UInt32 elem) const
    {
        return base_.get_property< ::AudioStreamBasicDescription>(
            kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, elem);
    }

    void input_format(::UInt32 elem, const ::AudioStreamBasicDescription& fmt)
    {
        base_.set_property(
            kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, elem, fmt);
    }

    void output_format(::UInt32 elem, const ::AudioStreamBasicDescription& fmt)
    {
        base_.set_property(
            kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, elem, fmt);
    }

    ::UInt32 device_buffer_size() const
    {
        return
            base_.get_property< ::UInt32>(kAudioDevicePropertyBufferFrameSize);
    }

    void set_render_callback(::AURenderCallback proc, void* ctx)
    {
        ::AURenderCallbackStruct data;
        data.inputProc = proc;
        data.inputProcRefCon = ctx;

        base_.set_property(
            kAudioUnitProperty_SetRenderCallback,
            kAudioUnitScope_Input, 0, data);
    }

    void set_input_callback(::AURenderCallback proc, void* ctx)
    {
        ::AURenderCallbackStruct data;
        data.inputProc = proc;
        data.inputProcRefCon = ctx;

        base_.set_property(kAudioOutputUnitProperty_SetInputCallback, data);
    }

    void enable_input(::UInt32 elem, bool on)
    {
        ::UInt32 data = on;
        base_.set_property(
            kAudioOutputUnitProperty_EnableIO,
            kAudioUnitScope_Input, elem, data);
    }

    void enable_output(::UInt32 elem, bool on)
    {
        ::UInt32 data = on;
        base_.set_property(
            kAudioOutputUnitProperty_EnableIO,
            kAudioUnitScope_Output, elem, data);
    }

    void render(
        ::AudioUnitRenderActionFlags* ioActionFlags,
        const ::AudioTimeStamp* inTimeStamp,
        ::UInt32 inBusNumber,
        ::UInt32 inNumberFrames,
        ::AudioBufferList* ioData)
    {
        base_.render(
            ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    }

private:
    audio_unit_base base_;
};

class audio_converter : private boost::noncopyable
{
public:
    audio_converter() : handle_(0)
    {
    }

    audio_converter(
        const ::AudioStreamBasicDescription& src,
        const ::AudioStreamBasicDescription& dst)
    {
        ::OSStatus err = ::AudioConverterNew(&src, &dst, &handle_);
        if (err != 0)
            throw BOOST_IOSTREAMS_FAILURE("failed AudioConverterNew()");
    }

    ~audio_converter()
    {
        if (handle_ != 0)
            ::AudioConverterDispose(handle_);
    }

    void swap(audio_converter& rhs)
    {
        std::swap(handle_, rhs.handle_);
    }

    ::OSStatus fill_buffer(
        ::AudioConverterInputDataProc proc, void* ctx,
        ::UInt32& size, void* buffer)
    {
        return ::AudioConverterFillBuffer(handle_, proc, ctx, &size, buffer);
    }

private:
    ::AudioConverterRef handle_;
};

class aligned_buffer : private boost::noncopyable
{
public:
    aligned_buffer() : ptr_(0)
    {
    }

    explicit aligned_buffer(std::size_t size)
        : ptr_(::operator new(size))
    {
    }

    ~aligned_buffer()
    {
        ::operator delete(ptr_);
    }

    void* address() const
    {
        return ptr_;
    }

    void swap(aligned_buffer& rhs)
    {
        std::swap(ptr_, rhs.ptr_);
    }

private:
    void* ptr_;
};

class audio_buffer_list : private boost::noncopyable
{
public:
    audio_buffer_list()
    {
    }

    audio_buffer_list(::UInt32 channel, ::UInt32 size)
        : data_(channel*size)
    {
        list_.mNumberBuffers = 1;

        ::AudioBuffer& buf = list_.mBuffers[0];
        buf.mNumberChannels = channel;
        buf.mDataByteSize = channel*size;
        buf.mData = static_cast<char*>(data_.address());
    }

    ::AudioBufferList* address()
    {
        return &list_;
    }

    void swap(audio_buffer_list& rhs)
    {
        std::swap(list_, rhs.list_);
        data_.swap(rhs.data_);
    }

private:
    ::AudioBufferList list_;
    aligned_buffer data_;
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

class output_queue
{
public:
    explicit output_queue(std::size_t buffer_size)
        : buffer_(buffer_size)
    {
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        mutex::scoped_lock lock(mutex_);
        while (buffer_.full())
            cond_.wait(mutex_);

        std::streamsize amt = hamigaki::auto_min(
            buffer_.capacity()-buffer_.size(), n);
        buffer_.insert(buffer_.end(), s, s+amt);

        return amt;
    }

    void flush()
    {
        mutex::scoped_lock lock(mutex_);
        while (!buffer_.empty())
            cond_.wait(mutex_);
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        mutex::scoped_lock lock(mutex_);

        std::streamsize amt = hamigaki::auto_min(buffer_.size(), n);
        if (amt != 0)
        {
            typedef detail::circular_buffer<char>::array_range array_range;

            const array_range& a1 = buffer_.array_one();
            std::streamsize amt1 = hamigaki::auto_min(a1.second, amt);
            std::memcpy(s, a1.first, amt1);

            if (amt1 != amt)
            {
                const array_range& a2 = buffer_.array_two();
                std::memcpy(s+amt1, a2.first, amt-amt1);
            }

            buffer_.rresize(buffer_.size()-amt);
            cond_.notify_one();
        }
        return amt;
    }

private:
    mutex mutex_;
    condition cond_;
    detail::circular_buffer<char> buffer_;
};

class input_queue
{
public:
    explicit input_queue(std::size_t buffer_size)
        : buffer_(buffer_size), failed_(false)
    {
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        mutex::scoped_lock lock(mutex_);
        while (buffer_.empty() && !failed_)
            cond_.wait(mutex_);
        if (failed_)
            throw BOOST_IOSTREAMS_FAILURE("bad read");

        std::streamsize amt = hamigaki::auto_min(buffer_.size(), n);

        typedef detail::circular_buffer<char>::array_range array_range;

        const array_range& a1 = buffer_.array_one();
        std::streamsize amt1 = hamigaki::auto_min(a1.second, amt);
        std::memcpy(s, a1.first, amt1);

        if (amt1 != amt)
        {
            const array_range& a2 = buffer_.array_two();
            std::memcpy(s+amt1, a2.first, amt-amt1);
        }

        buffer_.rresize(buffer_.size()-amt);

        return amt;
    }

    std::streamsize write(const char* s, std::streamsize n)
    {
        mutex::scoped_lock lock(mutex_);

        std::streamsize amt = hamigaki::auto_min(
            buffer_.capacity()-buffer_.size(), n);
        if (amt != 0)
        {
            buffer_.insert(buffer_.end(), s, s+amt);
            cond_.notify_one();
        }
        return amt;
    }

    void fail()
    {
        mutex::scoped_lock lock(mutex_);
        failed_ = true;
        cond_.notify_one();
    }

private:
    mutex mutex_;
    condition cond_;
    detail::circular_buffer<char> buffer_;
    bool failed_;
};

} // namespace

class audio_unit_sink::impl
{
public:
    impl(const pcm_format& f, std::size_t buffer_size)
        : output_((find_default_audio_output()))
        , format_(f)
        , queue_(buffer_size)
        , running_(false)
    {
        output_.set_render_callback(&impl::render_callback, this);
        output_.input_format(0, make_au_format(f));
    }

    ~impl()
    {
        if (running_)
            output_.stop();
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
        queue_.flush();
        output_.stop();
        running_ = false;
    }

    bool flush()
    {
        queue_.flush();
        return true;
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    audio_unit output_;
    pcm_format format_;
    output_queue queue_;
    bool running_;

    ::OSStatus render_callback_impl(
        ::AudioUnitRenderActionFlags* ioActionFlags,
        const ::AudioTimeStamp* inTimeStamp,
        ::UInt32 inBusNumber,
        ::UInt32 inNumberFrames,
        ::AudioBufferList* ioData)
    {
        std::streamsize n = ioData->mBuffers[0].mDataByteSize;
        char* s = static_cast<char*>(ioData->mBuffers[0].mData);

        std::streamsize amt = queue_.read(s, n);
        if (amt != n)
            std::memset(s+amt, 0, n-amt);

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
        try
        {
            return static_cast<impl*>(inRefCon)->render_callback_impl(
                ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData
            );
        }
        catch (...)
        {
        }
        return failed_render_callback;
    }
};

class audio_unit_source::impl
{
public:
    impl(const pcm_format& f, std::size_t buffer_size)
        : input_((find_auhal_output()))
        , format_(f)
        , queue_(buffer_size)
        , convert_buffer_size_(0)
        , running_(false)
    {
        input_.enable_input(1, true);
        input_.enable_output(0, false);

        input_.device_id(get_default_input_device());

        ::AudioStreamBasicDescription dst_fmt = make_au_format(f);
        ::AudioStreamBasicDescription src_fmt = dst_fmt;
        src_fmt.mSampleRate = input_.input_format(1).mSampleRate;
        input_.output_format(1, src_fmt);

        input_.set_input_callback(&impl::input_callback, this);

        audio_buffer_list buffers(
            f.channels,
            input_.device_buffer_size()*f.block_size()
        );
        buffers_.swap(buffers);

        if (src_fmt.mSampleRate != dst_fmt.mSampleRate)
        {
            audio_converter converter(src_fmt, dst_fmt);
            converter_.swap(converter);

            convert_buffer_size_ = static_cast< ::UInt32>(
                static_cast<double>(input_.device_buffer_size()) *
                (dst_fmt.mSampleRate/src_fmt.mSampleRate)
            );
            convert_buffer_size_ *= f.block_size();

            aligned_buffer buffer(convert_buffer_size_);
            convert_buffer_.swap(buffer);
        }
    }

    ~impl()
    {
        if (running_)
            input_.stop();
    }

    std::streamsize read(char* s, std::streamsize n)
    {
        if (!running_)
        {
            input_.start();
            running_ = true;
        }

        std::streamsize total = 0;
        while (total != n)
        {
            std::streamsize amt = queue_.read(s+total, n-total);
            total += amt;
        }

        return total;
    }

    void close()
    {
        input_.stop();
        running_ = false;
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    audio_unit input_;
    pcm_format format_;
    input_queue queue_;
    audio_buffer_list buffers_;
    audio_converter converter_;
    aligned_buffer convert_buffer_;
    ::UInt32 convert_buffer_size_;
    ::UInt32 convert_offset_;
    bool running_;

    ::OSStatus input_callback_impl(
        ::AudioUnitRenderActionFlags* ioActionFlags,
        const ::AudioTimeStamp* inTimeStamp,
        ::UInt32 inBusNumber,
        ::UInt32 inNumberFrames,
        ::AudioBufferList* ioData)
    {
        try
        {
            ::AudioBufferList* list_ptr = buffers_.address();

            list_ptr->mBuffers[0].mDataByteSize =
                inNumberFrames * format_.block_size();

            input_.render(
                ioActionFlags, inTimeStamp,
                inBusNumber, inNumberFrames, list_ptr);

            if (convert_buffer_size_ != 0)
            {
                convert_offset_ = 0;
                while (convert_offset_ != list_ptr->mBuffers[0].mDataByteSize)
                {
                    ::UInt32 size = convert_buffer_size_;
                    ::OSStatus err = converter_.fill_buffer(
                        &impl::convert_callback, this,
                        size, convert_buffer_.address());
                    if ((err != 0) && (err != need_more_input))
                        return err;

                    queue_.write(
                        static_cast<char*>(convert_buffer_.address()), size);
                }
            }
            else
            {
                const char* s = static_cast<char*>(list_ptr->mBuffers[0].mData);
                std::streamsize n = list_ptr->mBuffers[0].mDataByteSize;
                queue_.write(s, n);
            }

            return noErr;
        }
        catch (...)
        {
            queue_.fail();
        }
        return failed_input_callback;
    }

    static ::OSStatus input_callback(
        void* inRefCon,
        ::AudioUnitRenderActionFlags* ioActionFlags,
        const ::AudioTimeStamp* inTimeStamp,
        ::UInt32 inBusNumber,
        ::UInt32 inNumberFrames,
        ::AudioBufferList* ioData)
    {
        return static_cast<impl*>(inRefCon)->input_callback_impl(
            ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, ioData);
    }

    ::OSStatus convert_callback_impl(
        ::AudioConverterRef /*inAudioConverter*/,
        ::UInt32* ioDataSize, void** outData)
    {
        ::AudioBuffer& buf = buffers_.address()->mBuffers[0];

        ::UInt32 amt =
            (std::min)(*ioDataSize, buf.mDataByteSize - convert_offset_);

        if (amt != 0)
        {
            *ioDataSize = amt;
            *outData = static_cast<char*>(buf.mData) + convert_offset_;
            convert_offset_ += amt;
            return 0;
        }
        else
        {
            *ioDataSize = 0;
            *outData = 0;
            return need_more_input;
        }
    }

    static ::OSStatus convert_callback(
        ::AudioConverterRef inAudioConverter,
        ::UInt32* ioDataSize, void** outData, void* inUserData)
    {
        return static_cast<impl*>(inUserData)->
            convert_callback_impl(inAudioConverter, ioDataSize, outData);
    }
};

audio_unit_sink::audio_unit_sink(const pcm_format& f)
    : pimpl_(new impl(f, f.optimal_buffer_size()))
{
}

audio_unit_sink::audio_unit_sink(const pcm_format& f, std::size_t buffer_size)
    : pimpl_(new impl(f, buffer_size))
{
}

pcm_format audio_unit_sink::format() const
{
    return pimpl_->format();
}

std::streamsize audio_unit_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

bool audio_unit_sink::flush()
{
    return pimpl_->flush();
}

void audio_unit_sink::close()
{
    pimpl_->close();
}

audio_unit_source::audio_unit_source(const pcm_format& f)
    : pimpl_(new impl(f, f.optimal_buffer_size()))
{
}

audio_unit_source::audio_unit_source(
    const pcm_format& f, std::size_t buffer_size)
    : pimpl_(new impl(f, buffer_size))
{
}

pcm_format audio_unit_source::format() const
{
    return pimpl_->format();
}

std::streamsize audio_unit_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void audio_unit_source::close()
{
    pimpl_->close();
}

} } // End namespaces audio, hamigaki.
