// pcm_source_test.cpp: test case for pcm_source

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/pcm_device.hpp>
#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/copy.hpp>
#include <hamigaki/iostreams/tiny_restrict.hpp>
#include <boost/test/unit_test.hpp>
#include "detail/utility.hpp"

namespace audio = hamigaki::audio;
namespace io_ex = hamigaki::iostreams;
namespace io = boost::iostreams;
namespace ut = boost::unit_test;

using namespace hamigaki::audio::test;

void pcm_source_test_aux(const audio::pcm_format& fmt)
{
    std::ostringstream os;
    os
        << "rate=" << fmt.rate
        << ", bits=" << fmt.bits()
        << ", channels=" << fmt.channels;
    BOOST_CHECKPOINT(os.str());

    io::copy(
        io_ex::tiny_restrict(
            audio::pcm_source(fmt),
            fmt.block_size()*calc_samples_per_note(fmt.rate,150)
        ),
        io::null_sink()
    );
}

void pcm_source_test()
{
    const unsigned rates[] = { 11025, 22050, 44100 };
    const audio::sample_format_type types[] =
    {
        audio::uint8,
        audio::int_le16
    };

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
                pcm_source_test_aux(fmt);
            }
        }
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("pcm_source test");
    test->add(BOOST_TEST_CASE(&pcm_source_test));
    return test;
}
