// asio_device_impl.hpp: declaration of asio_device::impl

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_ASIO_DEVICE_IMPL_HPP
#define HAMIGAKI_AUDIO_ASIO_DEVICE_IMPL_HPP

#include <hamigaki/audio/asio.hpp>
#include <hamigaki/uuid.hpp>
#include <boost/detail/atomic_count.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

#include <hamigaki/detail/virtual_memory.hpp>
#include "detail/iasiodrv.hpp"

namespace hamigaki { namespace audio {

class asio_device::impl : boost::noncopyable
{
private:
    typedef boost::shared_ptr<asio_source::impl> source_ptr;
    typedef boost::shared_ptr<asio_sink::impl> sink_ptr;

public:
    explicit impl(const uuid& clsid, void* hwnd=0);
    ~impl();

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

    void start();
    void stop();
    bool release();

private:
    boost::shared_ptr< ::IASIO> pimpl_;
    std::vector<source_ptr> sources_;
    std::vector<sink_ptr> sinks_;
    std::vector< ::ASIOBufferInfo> info_;
    hamigaki::detail::virtual_memory thunks_;
    ::ASIOCallbacks callbacks_;
    std::streamsize buffer_size_;
    boost::detail::atomic_count start_count_;
    boost::detail::atomic_count stop_count_;

    void buffer_switch(long index, ::ASIOBool direct);
    void sample_rate_changed(::ASIOSampleRate rate);
    long asio_message(long selector, long value, void* message, double* opt);
    ::ASIOTime* buffer_switch_time_info(
        ::ASIOTime* params, long index, ::ASIOBool direct);

    static void buffer_switch_helper(
        impl* this_ptr, long index, ::ASIOBool direct);

    static void sample_rate_changed_helper(
        impl* this_ptr, ::ASIOSampleRate rate);

    static long asio_message_helper(impl* this_ptr,
        long selector, long value, void* message, double* opt);

    static ::ASIOTime* buffer_switch_time_info_helper(impl* this_ptr,
        ::ASIOTime* params, long index, ::ASIOBool direct);

    void init_thunks();

    sample_format_type get_sample_type(bool input, long index);
    std::streamsize preferred_buffer_size() const;
};

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_ASIO_DEVICE_IMPL_HPP
