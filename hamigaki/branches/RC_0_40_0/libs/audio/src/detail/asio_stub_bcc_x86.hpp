// asio_stub_bcc_x86.hpp: ASIO API stub functions for Borland/x86

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_BCC_X86_HPP
#define HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_BCC_X86_HPP

#include "./iasiodrv.hpp"

namespace hamigaki { namespace audio { namespace detail {

inline ::ASIOBool asio_init(::IASIO*, void*)
{
    __emit__(0x8B, 0x45, 0x0C);
    __emit__(0x50            );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x11      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x0C);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

inline ::ASIOError asio_start(::IASIO*)
{
    __emit__(0x8B, 0x45, 0x08);
    __emit__(0x8B, 0x10      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x1C);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

inline ::ASIOError asio_stop(::IASIO*)
{
    __emit__(0x8B, 0x45, 0x08);
    __emit__(0x8B, 0x10      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x20);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

inline ::ASIOError asio_get_buffer_size(::IASIO*, long*, long*, long*, long*)
{
    __emit__(0x8B, 0x45, 0x18);
    __emit__(0x50            );
    __emit__(0x8B, 0x4D, 0x14);
    __emit__(0x51            );
    __emit__(0x8B, 0x55, 0x10);
    __emit__(0x52            );
    __emit__(0x8B, 0x45, 0x0C);
    __emit__(0x50            );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x11      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x2C);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

inline ::ASIOError asio_get_sample_rate(::IASIO*, ::ASIOSampleRate*)
{
    __emit__(0x8B, 0x45, 0x0C);
    __emit__(0x50            );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x11      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x34);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

inline ::ASIOError asio_set_sample_rate(::IASIO*, ::ASIOSampleRate)
{
    __emit__(0x83, 0xEC, 0x08);
    __emit__(0xDD, 0x45, 0x0C);
    __emit__(0xDD, 0x1C, 0x24);
    __emit__(0x8B, 0x45, 0x08);
    __emit__(0x8B, 0x10      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x38);
    __emit__(0xFF, 0xD0      );
    __emit__(0x83, 0xEC, 0x08);
    __emit__(0xDD, 0x45, 0x0C);
    __emit__(0xDD, 0x1C, 0x24);
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x11      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x38);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

inline ::ASIOError asio_get_channel_info(::IASIO*, ::ASIOChannelInfo*)
{
    __emit__(0x8B, 0x45, 0x0C);
    __emit__(0x50            );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x11      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x48);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

inline ::ASIOError asio_create_buffers(::IASIO*,
    ::ASIOBufferInfo*, long, long, ::ASIOCallbacks*)
{
    __emit__(0x8B, 0x45, 0x18);
    __emit__(0x50            );
    __emit__(0x8B, 0x4D, 0x14);
    __emit__(0x51            );
    __emit__(0x8B, 0x55, 0x10);
    __emit__(0x52            );
    __emit__(0x8B, 0x45, 0x0C);
    __emit__(0x50            );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x11      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x4C);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

inline ::ASIOError asio_output_ready(::IASIO*)
{
    __emit__(0x8B, 0x45, 0x08);
    __emit__(0x8B, 0x10      );
    __emit__(0x8B, 0x4D, 0x08);
    __emit__(0x8B, 0x42, 0x5C);
    __emit__(0xFF, 0xD0      );
    return _EAX;
}

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_ASIO_STUB_BCC_X86_HPP
