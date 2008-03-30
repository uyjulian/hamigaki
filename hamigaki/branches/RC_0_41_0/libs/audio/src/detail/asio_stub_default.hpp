// asio_stub_default.hpp: ASIO API stub functions

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_DEFAULT_HPP
#define HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_DEFAULT_HPP

#include "./iasiodrv.hpp"

namespace hamigaki { namespace audio { namespace detail {

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

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_DEFAULT_HPP
