// pcm_format.hpp: pcm format infomation

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_PCM_FORMAT_HPP
#define HAMIGAKI_AUDIO_PCM_FORMAT_HPP

#include <hamigaki/audio/sample_format.hpp>

namespace hamigaki { namespace audio {

struct pcm_format
{
    sample_format_type type;
    int channels;
    long rate;

    int bits() const
    {
        return sample_bits(type);
    }

    std::streamsize block_size() const
    {
        return sample_size(type) * channels;
    }

    std::streamsize optimal_buffer_size() const
    {
        // 200msec
        return (rate / 5) * block_size();
    }
};

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_PCM_FORMAT_HPP
