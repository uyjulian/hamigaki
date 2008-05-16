// asio_stub_gcc_x86.hpp: ASIO API stub functions for gcc/x86

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_GCC_X86_HPP
#define HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_GCC_X86_HPP

#include "./iasiodrv.hpp"

namespace hamigaki { namespace audio { namespace detail {

inline ::ASIOBool asio_init(::IASIO* this_ptr, void* sys_handle)
{
    ::ASIOBool result;
    __asm__ __volatile__
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
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

inline ::ASIOError asio_start(::IASIO* this_ptr)
{
    ::ASIOError result;
    __asm__ __volatile__
    (
        "mov %1, %%eax\n\t"
        "mov (%%eax), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x1c(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr) :
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

inline ::ASIOError asio_stop(::IASIO* this_ptr)
{
    ::ASIOError result;
    __asm__ __volatile__
    (
        "mov %1, %%eax\n\t"
        "mov (%%eax), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x20(%%edx), %%eax\n\t"
        "call *%%eax" : 
        "=a" (result) :
        "m" (this_ptr) :
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

inline ::ASIOError asio_get_buffer_size(::IASIO* this_ptr,
    long* min_size, long* max_size, long* preferred_size, long* granularity)
{
    ::ASIOError result;
    __asm__ __volatile__
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
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

inline ::ASIOError asio_get_sample_rate(
    ::IASIO* this_ptr, ::ASIOSampleRate* sample_rate)
{
    ::ASIOError result;
    __asm__ __volatile__
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
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

inline ::ASIOError asio_set_sample_rate(
    ::IASIO* this_ptr, ::ASIOSampleRate sample_rate)
{
    ::ASIOError result;
    __asm__ __volatile__
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
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

inline ::ASIOError asio_get_channel_info(
    ::IASIO* this_ptr, ::ASIOChannelInfo* info)
{
    ::ASIOError result;
    __asm__ __volatile__
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
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

inline ::ASIOError asio_create_buffers(::IASIO* this_ptr,
    ::ASIOBufferInfo* buffer_infos, long num_channels,
    long buffer_size, ::ASIOCallbacks* callbacks)
{
    ::ASIOError result;
    __asm__ __volatile__
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
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

inline ::ASIOError asio_output_ready(::IASIO* this_ptr)
{
    ::ASIOError result;
    __asm__ __volatile__
    (
        "mov %1, %%eax\n\t"
        "mov (%%eax), %%edx\n\t"
        "mov %1, %%ecx\n\t"
        "mov 0x5c(%%edx), %%eax\n\t"
        "call *%%eax" :
        "=a" (result) :
        "m" (this_ptr) :
        "%ecx", "%edx", "cc", "memory"
    );
    return result;
}

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_GCC_X86_HPP
