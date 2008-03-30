// direct_sound_test.cpp: test case for DirectSound

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/amplify.hpp>
#include <hamigaki/audio/direct_sound.hpp>
#include <hamigaki/audio/sine_wave.hpp>
#include <hamigaki/audio/stereo.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <hamigaki/iostreams/tiny_restrict.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/test/unit_test.hpp>
#include "detail/utility.hpp"

namespace audio = hamigaki::audio;
namespace ds = audio::direct_sound;
namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

using namespace hamigaki::audio::test;

void direct_sound_buffer_test_aux(const audio::pcm_format& fmt, float freq)
{
    std::ostringstream os;
    os
        << "rate=" << fmt.rate
        << ", bits=" << fmt.bits()
        << ", channels=" << fmt.channels;
    BOOST_CHECKPOINT(os.str());

    audio::direct_sound_device dev;
    dev.set_cooperative_level(0, ds::priority_level);
    dev.format(fmt);
    io::copy(
        io_ex::tiny_restrict(
            audio::stereo(
                audio::amplify(
                    audio::sine_wave_source(fmt.rate, freq),
                    0.5f
                ),
                fmt.channels
            ), fmt.channels*calc_samples_per_note(fmt.rate,150)
        ),
        audio::widen<float>(dev.create_buffer(fmt, fmt.optimal_buffer_size()))
    );
}

void direct_sound_buffer_test()
{
    const unsigned rates[] = { 11025, 22050, 44100 };
    const audio::sample_format_type types[] =
    {
        audio::uint8,
        audio::int_le16
    };

    unsigned short note = 60;
    for (std::size_t i = 0; i < sizeof(rates)/sizeof(rates[0]); ++i)
    {
        for (std::size_t j = 0; j < sizeof(types)/sizeof(types[0]); ++j)
        {
            for (unsigned channels = 1; channels <= 2; ++channels)
            {
                audio::pcm_format fmt;
                fmt.rate = rates[i];
                fmt.type = types[j];
                fmt.channels = channels;
                direct_sound_buffer_test_aux(fmt, calc_frequency(note));
            }
            note = next_note(note);
        }
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("DirectSound test");
    test->add(BOOST_TEST_CASE(&direct_sound_buffer_test));
    return test;
}
