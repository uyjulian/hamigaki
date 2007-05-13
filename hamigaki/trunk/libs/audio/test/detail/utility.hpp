// utility.hpp: utility for Hamigaki.Audio tests

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_TEST_UTILITY_HPP
#define HAMIGAKI_AUDIO_TEST_UTILITY_HPP

#include <cmath>

namespace hamigaki { namespace audio { namespace test {

namespace detail
{

inline float power(float x, float y)
{
    if (y >= 0.0f)
        return std::pow(x, y);
    else
        return std::pow(1.0f/x, -y);
}

} // namespace detail

// calculate the frequency from specified MIDI note number
inline float calc_frequency(unsigned short note)
{
    return 440.0f * detail::power(2.0f, (note-69) / 12.0f);
}

inline unsigned calc_samples_per_note(unsigned rate, unsigned tempo)
{
    return rate * 60 / tempo;
}

inline unsigned short next_note(unsigned short note)
{
    unsigned offset = note % 12;
    if ((offset == 4) || (offset == 11))
        return static_cast<unsigned short>(note + 1);
    else
        return static_cast<unsigned short>(note + 2);
}

} } } // End namespaces test, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_TEST_UTILITY_HPP
