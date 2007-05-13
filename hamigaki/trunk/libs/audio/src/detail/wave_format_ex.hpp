// wave_format_ex.hpp: WAVEFORMATEX wrapper

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_WAVE_FORMAT_EX_HPP
#define HAMIGAKI_AUDIO_DETAIL_WAVE_FORMAT_EX_HPP

#include <hamigaki/audio/pcm_format.hpp>
#include <boost/assert.hpp>
#include <windows.h>
#include <mmsystem.h>

namespace hamigaki { namespace audio { namespace detail {

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
        nAvgBytesPerSec = f.rate * f.block_size();
        nBlockAlign = static_cast<unsigned short>(f.block_size());
        wBitsPerSample = static_cast<unsigned short>(f.bits());
        cbSize = 0;
    }
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_WAVE_FORMAT_EX_HPP
