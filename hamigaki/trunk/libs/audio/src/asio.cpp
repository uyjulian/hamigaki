//  asio.cpp: ASIO devices

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#include "asio_sink_impl.hpp"
#include "asio_source_impl.hpp"
#include <boost/assert.hpp>

#include <hamigaki/detail/windows/com_release.hpp>
#include <hamigaki/detail/cdecl_thunk.hpp>

namespace hamigaki { namespace audio {

namespace
{

using namespace hamigaki::detail::windows;

const std::size_t buffer_count = 4;

struct asio_callbacks
{
    hamigaki::detail::cdecl_thunk<2> buffer_switch;
    hamigaki::detail::cdecl_thunk<2> sample_rate_changed;
    hamigaki::detail::cdecl_thunk<4> asio_message;
    hamigaki::detail::cdecl_thunk<3> buffer_switch_time_info;
};

#if !defined(BOOST_NO_STD_WSTRING)
const ::CLSID clsid_from_string(const std::wstring& s)
{
    ::CLSID id;
    ::CLSIDFromString(const_cast<wchar_t*>(s.c_str()), &id);
    return id;
}
#endif
const ::CLSID clsid_from_string(const std::string& s)
{
    std::vector<wchar_t> wide;
    wide.reserve(s.size()+1);
    for (std::size_t i = 0; i < s.size(); ++i)
        wide.push_back(static_cast<wchar_t>(s[i]));
    wide.push_back(L'\0');

    ::CLSID id;
    ::CLSIDFromString(&wide[0], &id);
    return id;
}

boost::shared_ptr< ::IASIO> create_asio(const ::CLSID& clsid)
{
    void* tmp = 0;
    ::HRESULT res = ::CoCreateInstance(
        clsid, 0, CLSCTX_INPROC_SERVER, clsid, &tmp);
    if (FAILED(res))
        throw std::runtime_error("cannot create ASIO");

    ::IASIO* ptr = static_cast< ::IASIO*>(tmp);
    return boost::shared_ptr< ::IASIO>(ptr, com_release());
}

#if defined(__GNUC__)
inline ::ASIOBool asio_init(::IASIO* this_ptr, void *sys_handle)
{
    ::ASIOBool result;
    __asm__
    (
        "mov %2, %%eax\n\t"
        "push %%eax\n\t"
        "mov %1, %%ecx\n\t"
        "mov (%%ecx), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0xc(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr), "m" (sys_handle) :
        "%ecx", "%edx"
    );
    return result;
}

inline ::ASIOError asio_start(::IASIO* this_ptr)
{
    ::ASIOError result;
    __asm__
    (
        "mov %1, %%eax\n\t"
        "mov (%%eax), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x1c(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr) :
        "%ecx", "%edx"
    );
    return result;
}

inline ::ASIOError asio_stop(::IASIO* this_ptr)
{
    ::ASIOError result;
    __asm__
    (
        "mov %1, %%eax\n\t"
        "mov (%%eax), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x20(%%edx), %%eax\n\t"
        "call *%%eax" : 
        "=a" (result) :
        "m" (this_ptr) :
        "%ecx", "%edx"
    );
    return result;
}

inline ::ASIOError asio_get_buffer_size(::IASIO* this_ptr,
    long* min_size, long* max_size, long* preferred_size, long* granularity)
{
    ::ASIOError result;
    __asm__
    (
        "mov %5, %%eax\n\t"
        "push %%eax\n\t"
        "mov %4, %%ecx\n\t"
        "push %%ecx\n\t"
        "mov %3, %%edx\n\t"
        "push %%edx\n\t"
        "mov %2, %%eax\n\t"
        "push %%eax\n\t"
        "mov %1, %%ecx\n\t"
        "mov (%%ecx), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x2c(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr), "m" (min_size), "m" (max_size),
        "m" (preferred_size), "m" (granularity) :
        "%ecx", "%edx"
    );
    return result;
}

inline ::ASIOError asio_get_sample_rate(
    ::IASIO* this_ptr, ::ASIOSampleRate* sample_rate)
{
    ::ASIOError result;
    __asm__
    (
        "mov %2, %%eax\n\t"
        "push %%eax\n\t"
        "mov %1, %%ecx\n\t"
        "mov (%%ecx), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x34(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr), "m" (sample_rate) :
        "%ecx", "%edx"
    );
    return result;
}

inline ::ASIOError asio_set_sample_rate(
    ::IASIO* this_ptr, ::ASIOSampleRate sample_rate)
{
    ::ASIOError result;
    __asm__
    (
        "sub $0x8, %%esp\n\t"
        "fldl %2\n\t"
        "fstpl (%%esp)\n\t"
        "mov %1, %%eax\n\t"
        "mov (%%eax), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x38(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr), "m" (sample_rate) :
        "%ecx", "%edx"
    );
    return result;
}

inline ::ASIOError asio_get_channel_info(
    ::IASIO* this_ptr, ::ASIOChannelInfo* info)
{
    ::ASIOError result;
    __asm__
    (
        "mov %2, %%eax\n\t"
        "push %%eax\n\t"
        "mov %1, %%ecx\n\t"
        "mov (%%ecx), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x48(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr), "m" (info) :
        "%ecx", "%edx"
    );
    return result;
}

inline ::ASIOError asio_create_buffers(::IASIO* this_ptr,
    ::ASIOBufferInfo* buffer_infos, long num_channels,
    long buffer_size, ::ASIOCallbacks* callbacks)
{
    ::ASIOError result;
    __asm__
    (
        "mov %5, %%eax\n\t"
        "push %%eax\n\t"
        "mov %4, %%ecx\n\t"
        "push %%ecx\n\t"
        "mov %3, %%edx\n\t"
        "push %%edx\n\t"
        "mov %2, %%eax\n\t"
        "push %%eax\n\t"
        "mov %1, %%ecx\n\t"
        "mov (%%ecx), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x4c(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr), "m" (buffer_infos), "m" (num_channels),
        "m" (buffer_size), "m" (callbacks) :
        "%ecx", "%edx"
    );
    return result;
}

inline ::ASIOError asio_output_ready(::IASIO* this_ptr)
{
    ::ASIOError result;
    __asm__
    (
        "mov %1, %%eax\n\t"
        "mov (%%eax), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x5c(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr) :
        "%ecx", "%edx"
    );
    return result;
}
#else
inline ::ASIOBool asio_init(::IASIO* this_ptr, void* sys_handle)
{
    return this_ptr->init(sys_handle);
}

inline ::ASIOError asio_start(::IASIO* this_ptr)
{
    return this_ptr->start();
}

inline ::ASIOError asio_stop(::IASIO* this_ptr)
{
    return this_ptr->stop();
}

inline ::ASIOError asio_get_buffer_size(::IASIO* this_ptr,
    long* min_size, long* max_size, long* preferred_size, long* granularity)
{
    return this_ptr->getBufferSize(
        min_size, max_size, preferred_size, granularity);
}

inline ::ASIOError asio_get_sample_rate(
    ::IASIO* this_ptr, ::ASIOSampleRate* sample_rate)
{
    return this_ptr->getSampleRate(sample_rate);
}

inline ::ASIOError asio_set_sample_rate(
    ::IASIO* this_ptr, ::ASIOSampleRate sample_rate)
{
    return this_ptr->setSampleRate(sample_rate);
}

inline ::ASIOError asio_get_channel_info(
    ::IASIO* this_ptr, ::ASIOChannelInfo* info)
{
    return this_ptr->getChannelInfo(info);
}

inline ::ASIOError asio_create_buffers(::IASIO* this_ptr,
    ::ASIOBufferInfo* buffer_infos, long num_channels,
    long buffer_size, ::ASIOCallbacks* callbacks)
{
    return this_ptr->createBuffers(
        buffer_infos, num_channels, buffer_size, callbacks);
}

inline ::ASIOError asio_output_ready(::IASIO* this_ptr)
{
    return this_ptr->outputReady();
}
#endif

} // namespace

asio_source::impl::impl(asio_device::impl& dev,
        sample_format_type type, std::size_t buffer_size)
    : dev_ptr_(&dev), type_(type)
    , buffer_(buffer_size*buffer_count)
    , read_pos_(0), write_pos_(0), is_open_(true)
{
    for (std::size_t i = 0; i < buffer_count; ++i)
        events_.push_back(new auto_reset_event(false));
}

asio_source::impl::~impl()
{
    try
    {
        close();
    }
    catch (...)
    {
    }
}

std::streamsize asio_source::impl::read(char* s, std::streamsize n)
{
    std::streamsize total = 0;
    const std::size_t buffer_size = buffer_.size()/buffer_count;
    while (n != 0)
    {
        if ((read_pos_ % buffer_size) == 0)
        {
            std::size_t index = read_pos_ / buffer_size;
            events_[index].wait();
        }

        std::streamsize amt =
            std::min<std::streamsize>(n,
                buffer_size - read_pos_%buffer_size);

        std::memcpy(s, &buffer_[read_pos_], amt);

        s += amt;
        n -= amt;
        total += amt;
        read_pos_ += amt;
        read_pos_ %= buffer_.size();
    }
    return (total != 0) ? total : -1;
}

void asio_source::impl::close()
{
    if (!is_open_)
        return;

    {
        const critical_section::scoped_lock locking(cs_);
        is_open_ = false;
    }

    if (dev_ptr_->release())
        dev_ptr_->stop();
}

std::streamsize asio_source::impl::write(const char* s, std::streamsize n)
{
    const critical_section::scoped_lock locking(cs_);

    const std::size_t buffer_size = buffer_.size()/buffer_count;
    BOOST_ASSERT(static_cast<std::size_t>(n) == buffer_size);

    std::memcpy(&buffer_[write_pos_], s, n);

    events_[write_pos_/buffer_size].set();

    write_pos_ += n;
    write_pos_ %= buffer_.size();
    return n;
}

asio_source::asio_source(const boost::shared_ptr<impl>& pimpl) : pimpl_(pimpl)
{
}

std::streamsize asio_source::read(char* s, std::streamsize n)
{
    return pimpl_->read(s, n);
}

void asio_source::close()
{
    pimpl_->close();
}

sample_format_type asio_source::sample_format() const
{
    return pimpl_->sample_format();
}


asio_sink::impl::impl(asio_device::impl& dev,
        sample_format_type type, std::size_t buffer_size)
    : dev_ptr_(&dev), type_(type)
    , buffer_(buffer_size*buffer_count)
    , write_pos_(0), read_pos_(0)
    , is_open_(true), playing_(false), eof_(false)
{
    for (std::size_t i = 0; i < buffer_count; ++i)
        events_.push_back(new auto_reset_event);
}

asio_sink::impl::~impl()
{
    try
    {
        close();
    }
    catch (...)
    {
    }
}

std::streamsize asio_sink::impl::write(const char* s, std::streamsize n)
{
    std::streamsize total = 0;
    const std::size_t buffer_size = buffer_.size()/buffer_count;
    while (n != 0)
    {
        if ((write_pos_ % buffer_size) == 0)
        {
            std::size_t index = write_pos_ / buffer_size;
            events_[index].wait();
        }

        std::streamsize amt =
            std::min<std::streamsize>(n,
                buffer_size - write_pos_%buffer_size);

        std::memcpy(&buffer_[write_pos_], s, amt);

        s += amt;
        n -= amt;
        total += amt;
        write_pos_ += amt;
        write_pos_ %= buffer_.size();

        if (!playing_ && (write_pos_ == 0))
        {
            dev_ptr_->start();
            playing_ = true;
        }
    }
    return (total != 0) ? total : -1;
}

void asio_sink::impl::close()
{
    if (!is_open_)
        return;

    const std::size_t buffer_size = buffer_.size()/buffer_count;
    if (write_pos_ % buffer_size != 0)
    {
        std::streamsize amt = buffer_size - write_pos_%buffer_size;
        std::memset(&buffer_[write_pos_], 0, amt);
        write_pos_ += amt;
        write_pos_ %= buffer_.size();
    }

    {
        const critical_section::scoped_lock locking(cs_);
        is_open_ = false;
    }

    if (playing_ && dev_ptr_->release())
    {
        std::size_t index = write_pos_ / buffer_size;
        events_[(index + buffer_count - 1) % buffer_count].wait();
        dev_ptr_->stop();
    }
}

std::streamsize asio_sink::impl::read(char* s, std::streamsize n)
{
    const critical_section::scoped_lock locking(cs_);

    const std::size_t buffer_size = buffer_.size()/buffer_count;
    BOOST_ASSERT(static_cast<std::size_t>(n) == buffer_size);

    if (!is_open_ && !eof_ && (read_pos_ == write_pos_))
        eof_ = true;

    if (eof_)
        std::memset(s, 0, n);
    else
        std::memcpy(s, &buffer_[read_pos_], n);

    events_[read_pos_/buffer_size].set();

    read_pos_ += n;
    read_pos_ %= buffer_.size();
    return n;
}

void asio_sink::impl::fill()
{
    for (std::size_t i = 0; i < buffer_count; ++i)
        events_[i].wait();
    dev_ptr_->start();
    playing_ = true;
}

asio_sink::asio_sink(const boost::shared_ptr<impl>& pimpl) : pimpl_(pimpl)
{
}

std::streamsize asio_sink::write(const char* s, std::streamsize n)
{
    return pimpl_->write(s, n);
}

void asio_sink::close()
{
    pimpl_->close();
}

sample_format_type asio_sink::sample_format() const
{
    return pimpl_->sample_format();
}


#if !defined(BOOST_NO_STD_WSTRING)
asio_device::impl::impl(const std::wstring& clsid, void* hwnd)
    : pimpl_(create_asio(clsid_from_string(clsid)))
    , thunks_(sizeof(asio_callbacks)), start_count_(0), stop_count_(0)
{
    if (!asio_init(pimpl_.get(), hwnd))
        throw std::runtime_error("failed initialization of ASIO");

    init_thunks();

    buffer_size_ = preferred_buffer_size();
}
#endif
asio_device::impl::impl(const std::string& clsid, void* hwnd)
    : pimpl_(create_asio(clsid_from_string(clsid)))
    , thunks_(sizeof(asio_callbacks)), start_count_(0), stop_count_(0)
{
    if (!asio_init(pimpl_.get(), hwnd))
        throw std::runtime_error("failed initialization of ASIO");

    init_thunks();

    buffer_size_ = preferred_buffer_size();
}

asio_device::impl::~impl()
{
    for (std::size_t i = 0; i < sources_.size(); ++i)
        sources_[i]->close();

    for (std::size_t i = 0; i < sinks_.size(); ++i)
        sinks_[i]->close();
}

double asio_device::impl::rate() const
{
    double r;
    ::ASIOError err = asio_get_sample_rate(pimpl_.get(), &r);
    if (err != ASE_OK)
        throw std::runtime_error("failed IASIO::getSampleRate()");
    return r;
}

void asio_device::impl::rate(double r)
{
    ::ASIOError err = asio_set_sample_rate(pimpl_.get(), r);
    if (err != ASE_OK)
        throw std::runtime_error("failed IASIO::setSampleRate()");
}

void asio_device::impl::create_buffers(long in_channels, long out_channels)
{
    BOOST_ASSERT(info_.empty());
    BOOST_ASSERT(sources_.empty());
    BOOST_ASSERT(sinks_.empty());

    info_.reserve(in_channels+out_channels);
    for (long i = 0; i < in_channels; ++i)
    {
        ++stop_count_;

        ::ASIOBufferInfo info;
        info.isInput = ASIOTrue;
        info.channelNum = i;
        info.buffers[0] = 0;
        info.buffers[1] = 0;
        info_.push_back(info);

        sample_format_type type = get_sample_type(true, i);
        std::size_t smp_sz = sample_size(type);

        source_ptr tmp(
            new asio_source::impl(*this, type, smp_sz*buffer_size_));
        sources_.push_back(tmp);
    }
    for (long i = 0; i < out_channels; ++i)
    {
        ++start_count_;
        ++stop_count_;

        ::ASIOBufferInfo info;
        info.isInput = ASIOFalse;
        info.channelNum = i;
        info.buffers[0] = 0;
        info.buffers[1] = 0;
        info_.push_back(info);

        sample_format_type type = get_sample_type(false, i);
        std::size_t smp_sz = sample_size(type);

        sink_ptr tmp(
            new asio_sink::impl(*this, type, smp_sz*buffer_size_));
        sinks_.push_back(tmp);
    }

    ::ASIOError err = asio_create_buffers(pimpl_.get(),
        &info_[0], info_.size(), buffer_size_, &callbacks_);
    if (err != ASE_OK)
        throw std::runtime_error("cannot create ASIO buffers");

    if (in_channels != 0)
    {
        if (out_channels != 0)
        {
            for (long i = 0; i < out_channels; ++i)
                sinks_[i]->fill();
        }
        else
        {
            ++start_count_; // HACK
            start();
        }
    }
}

asio_source asio_device::impl::get_source(std::size_t idx)
{
    return asio_source(sources_[idx]);
}

asio_sink asio_device::impl::get_sink(std::size_t idx)
{
    return asio_sink(sinks_[idx]);
}

std::size_t asio_device::impl::source_channels() const
{
    return sources_.size();
}

std::size_t asio_device::impl::sink_channels() const
{
    return sinks_.size();
}

std::streamsize asio_device::impl::buffer_size() const
{
    return buffer_size_;
}

void asio_device::impl::buffer_size(std::streamsize n)
{
    buffer_size_ = n;
}

asio_buffer_info asio_device::impl::buffer_info() const
{
    asio_buffer_info info;

    ::ASIOError err = asio_get_buffer_size(pimpl_.get(),
        &info.min_size, &info.max_size,
        &info.preferred_size, &info.granularity);
    if (err != ASE_OK)
        throw std::runtime_error("failed IASIO::getBufferSize()");

    return info;
}

void asio_device::impl::start()
{
    if (--start_count_ == 0)
    {
        ::ASIOError err = asio_start(pimpl_.get());
        if (err != ASE_OK)
            throw std::runtime_error("cannot start ASIO");
    }
}

void asio_device::impl::stop()
{
    ::ASIOError err = asio_stop(pimpl_.get());
    if (err != ASE_OK)
        throw std::runtime_error("cannot stop ASIO");
}

bool asio_device::impl::release()
{
    return --stop_count_ == 0;
}

void asio_device::impl::buffer_switch(long index, ::ASIOBool direct)
{
    for (std::size_t i = 0; i < sources_.size(); ++i)
    {
        std::size_t n = sample_size(sources_[i]->sample_format())*buffer_size_;
        sources_[i]->write(
            static_cast<char*>(info_[i].buffers[index]), n);
    }

    for (std::size_t i = 0; i < sinks_.size(); ++i)
    {
        std::size_t n = sample_size(sinks_[i]->sample_format())*buffer_size_;
        sinks_[i]->read(
            static_cast<char*>(info_[sources_.size()+i].buffers[index]), n);
    }

    asio_output_ready(pimpl_.get());
}

void asio_device::impl::sample_rate_changed(::ASIOSampleRate rate)
{
}

long asio_device::impl::asio_message(
    long selector, long value, void* message, double* opt)
{
    if (selector == kAsioSelectorSupported)
        return value == kAsioEngineVersion;
    else if (selector == kAsioEngineVersion)
        return 2;
    return 0;
}

::ASIOTime* asio_device::impl::buffer_switch_time_info(
    ::ASIOTime* params, long index, ::ASIOBool direct)
{
    return 0;
}

void asio_device::impl::buffer_switch_helper(
    impl* this_ptr, long index, ::ASIOBool direct)
{
    try
    {
        this_ptr->buffer_switch(index, direct);
    }
    catch (...)
    {
    }
}

void asio_device::impl::sample_rate_changed_helper(
    impl* this_ptr, ::ASIOSampleRate rate)
{
    try
    {
        this_ptr->sample_rate_changed(rate);
    }
    catch (...)
    {
    }
}

long asio_device::impl::asio_message_helper(impl* this_ptr,
    long selector, long value, void* message, double* opt)
{
    try
    {
        return this_ptr->asio_message(selector, value, message, opt);
    }
    catch (...)
    {
    }
    return 0;
}

::ASIOTime* asio_device::impl::buffer_switch_time_info_helper(impl* this_ptr,
    ::ASIOTime* params, long index, ::ASIOBool direct)
{
    try
    {
        return this_ptr->buffer_switch_time_info(params, index, direct);
    }
    catch (...)
    {
    }
    return 0;
}

void asio_device::impl::init_thunks()
{
    using hamigaki::detail::func_ptr_cast;

    asio_callbacks* cbs_ptr =
        static_cast<asio_callbacks*>(thunks_.address());

    cbs_ptr->buffer_switch.set_instance(
        func_ptr_cast<void*>(&impl::buffer_switch_helper), this);
    cbs_ptr->buffer_switch.copy_address(callbacks_.bufferSwitch);

    cbs_ptr->sample_rate_changed.set_instance(
        func_ptr_cast<void*>(&impl::sample_rate_changed_helper), this);
    cbs_ptr->sample_rate_changed.
        copy_address(callbacks_.sampleRateDidChange);

    cbs_ptr->asio_message.set_instance(
        func_ptr_cast<void*>(&impl::asio_message_helper), this);
    cbs_ptr->asio_message.copy_address(callbacks_.asioMessage);

    cbs_ptr->buffer_switch_time_info.set_instance(
        func_ptr_cast<void*>(
            &impl::buffer_switch_time_info_helper), this);
    cbs_ptr->buffer_switch_time_info.
        copy_address(callbacks_.bufferSwitchTimeInfo);

    thunks_.flush_icache();

    using hamigaki::detail::virtual_memory;
    thunks_.protect(virtual_memory::execute|virtual_memory::read);
}

sample_format_type
asio_device::impl::get_sample_type(bool input, long index)
{
    ::ASIOChannelInfo info = {0,};
    info.channel = index;
    info.isInput = input ? ASIOTrue : ASIOFalse;
    ::ASIOError err = asio_get_channel_info(pimpl_.get(), &info);
    if (err != ASE_OK)
        throw std::runtime_error("bad ASIO channel number");

    if (info.type == ASIOSTInt16MSB)
        return int_be16;
    else if (info.type == ASIOSTInt16LSB)
        return int_le16;
    else if (info.type == ASIOSTInt24MSB)
        return int_be24;
    else if (info.type == ASIOSTInt24LSB)
        return int_le24;
    else if (info.type == ASIOSTInt32MSB)
        return int_be32;
    else if (info.type == ASIOSTInt32LSB)
        return int_le32;
    else if (info.type == ASIOSTInt32MSB16)
        return int_a4_be16;
    else if (info.type == ASIOSTInt32LSB16)
        return int_a4_le16;
    else if (info.type == ASIOSTInt32MSB18)
        return int_a4_be18;
    else if (info.type == ASIOSTInt32LSB18)
        return int_a4_le18;
    else if (info.type == ASIOSTInt32MSB20)
        return int_a4_be20;
    else if (info.type == ASIOSTInt32LSB20)
        return int_a4_le20;
    else if (info.type == ASIOSTInt32MSB24)
        return int_a4_be24;
    else if (info.type == ASIOSTInt32LSB24)
        return int_a4_le24;
    else if (info.type == ASIOSTFloat32MSB)
        return float_be32;
    else if (info.type == ASIOSTFloat32LSB)
        return float_le32;
    else if (info.type == ASIOSTFloat64MSB)
        return float_be64;
    else if (info.type == ASIOSTFloat64LSB)
        return float_le64;

    throw std::runtime_error("unsupported ASIO format");
    return static_cast<sample_format_type>(0); // dummy
}

std::streamsize asio_device::impl::preferred_buffer_size() const
{
    long min_size;
    long max_size;
    long preferred_size;
    long granularity;
    ::ASIOError err = asio_get_buffer_size(pimpl_.get(),
        &min_size, &max_size, &preferred_size, &granularity);
    if (err != ASE_OK)
        throw std::runtime_error("failed IASIO::getBufferSize()");
    return preferred_size;
}

#if !defined(BOOST_NO_STD_WSTRING)
asio_device::asio_device(const std::wstring& clsid, void* hwnd)
    : pimpl_(new impl(clsid, hwnd))
{
}
#endif

asio_device::asio_device(const std::string& clsid, void* hwnd)
    : pimpl_(new impl(clsid, hwnd))
{
}

double asio_device::rate() const
{
    return pimpl_->rate();
}

void asio_device::rate(double r)
{
    pimpl_->rate(r);
}

void asio_device::create_buffers(long in_channels, long out_channels)
{
    pimpl_->create_buffers(in_channels, out_channels);
}

asio_source asio_device::get_source(std::size_t idx)
{
    return pimpl_->get_source(idx);
}

asio_sink asio_device::get_sink(std::size_t idx)
{
    return pimpl_->get_sink(idx);
}

std::size_t asio_device::source_channels() const
{
    return pimpl_->source_channels();
}

std::size_t asio_device::sink_channels() const
{
    return pimpl_->sink_channels();
}

std::streamsize asio_device::buffer_size() const
{
    return pimpl_->buffer_size();
}

void asio_device::buffer_size(std::streamsize n)
{
    pimpl_->buffer_size(n);
}

asio_buffer_info asio_device::buffer_info() const
{
    return pimpl_->buffer_info();
}

} } // End namespaces audio, hamigaki.
