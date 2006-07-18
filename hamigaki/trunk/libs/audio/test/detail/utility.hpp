//  utility.hpp: utility for Hamigaki.Audio tests

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_AUDIO_TEST_UTILITY_HPP
#define HAMIGAKI_AUDIO_TEST_UTILITY_HPP

#include <cmath>

namespace hamigaki { namespace audio { namespace test {

namespace detail
{

inline double power(double x, double y)
{
    if (y >= 0.0)
        return std::pow(x, y);
    else
        return std::pow(1.0/x, -y);
}

} // namespace detail

// calculate the frequency from specified MIDI note number
inline double calc_frequency(unsigned short note)
{
    return 440.0 * detail::power(2.0, (note-69) / 12.0);
}

inline unsigned calc_samples_per_note(unsigned rate, unsigned tempo)
{
    return rate * 60 / tempo;
}

inline unsigned short next_note(unsigned short note)
{
    unsigned offset = note % 12;
    if ((offset == 4) || (offset == 11))
        return note + 1;
    else
        return note + 2;
}

} } } // End namespaces test, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_TEST_UTILITY_HPP
