// square_wave.hpp: square wave generator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_SQUARE_WAVE_HPP
#define HAMIGAKI_AUDIO_SQUARE_WAVE_HPP

#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <cmath>

namespace hamigaki { namespace audio {

template<class CharT=float>
class basic_square_wave_source
{
public:
    typedef CharT char_type;

    struct category
        : public boost::iostreams::input
        , public boost::iostreams::device_tag
    {};

    basic_square_wave_source(long rate, CharT freq)
        : rate_(rate), freq_(freq), sum_()
    {
    }

    std::streamsize read(CharT* s, std::streamsize n)
    {
        if (n <= 0)
            return -1;

        const CharT v = freq_ / static_cast<CharT>(rate_);

        for (std::streamsize i = 0; i < n; ++i)
        {
            *(s++) = (sum_ < 0.5f) ? 1.0f : -1.0f;
            sum_ += v;
            if (sum_ >= 1.0f)
                sum_ -= 1.0f;
        }
        return n;
    }

    long rate() const
    {
        return rate_;
    }

    CharT frequency() const
    {
        return freq_;
    }

private:
    long rate_;
    CharT freq_;
    CharT sum_;
};

typedef basic_square_wave_source<> square_wave_source;

} } // End namespaces audio, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::audio::basic_square_wave_source, 1)

#endif // HAMIGAKI_AUDIO_SQUARE_WAVE_HPP
