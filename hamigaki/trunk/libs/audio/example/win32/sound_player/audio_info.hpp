// audio_info.hpp: audio file infomation for sound_player

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef AUDIO_INFO_HPP
#define AUDIO_INFO_HPP

#include <boost/cstdint.hpp>
#include <string>

struct audio_info
{
    std::string container;
    std::string encoding;
    boost::int64_t length;
    boost::uint32_t bit_rate;
    unsigned bits;
    int sampling_rate;
    unsigned channels;
};

#endif // AUDIO_INFO_HPP
