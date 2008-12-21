// frog_round.cpp: an example for multi-buffer mixing

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/version.hpp>

#include <hamigaki/audio/amplify.hpp>
#include <hamigaki/audio/square_wave.hpp>
#include <hamigaki/audio/stereo.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <hamigaki/iostreams/catable/restrict.hpp>
#include <hamigaki/iostreams/tiny_restrict.hpp>
#include <hamigaki/thread/utc_time.hpp>
#include <hamigaki/dec_format.hpp>
#include <boost/iostreams/copy.hpp>
#if BOOST_WORKAROUND(BOOST_VERSION, == 103800)
    #include <boost/date_time/date_defs.hpp> // kepp above thread.hpp
#endif
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "../test/detail/utility.hpp"

#if defined(BOOST_WINDOWS)
    #include <hamigaki/audio/direct_sound.hpp>
    typedef hamigaki::audio::direct_sound_sink sink_type;
#else
    #include <hamigaki/audio/pulse_audio.hpp>
    typedef hamigaki::audio::pulse_audio_sink sink_type;
#endif

namespace audio = hamigaki::audio;
namespace io_ex = hamigaki::iostreams;
namespace thread_ex = hamigaki::thread;
namespace io = boost::iostreams;

using namespace hamigaki::audio::test;

const int tempo = 100;

#define NOTE(n, len) \
    ( \
        io_ex::tiny_restrict( \
            audio::stereo( \
                audio::amplify( \
                    audio::square_wave_source( \
                        fmt.rate, calc_frequency(n) \
                    ), \
                    0.25f \
                ), \
                fmt.channels \
            ), \
            2*(calc_samples_per_note(fmt.rate,tempo)/2)*(len) \
        ) \
    )

#define C NOTE(60,1)
#define D NOTE(62,1)
#define E NOTE(64,1)
#define F NOTE(65,1)

#define CC (C + C)
#define DD (D + D)
#define EE (E + E)
#define FF (F + F)

#define C4 NOTE(60,2)
#define D4 NOTE(62,2)
#define E4 NOTE(64,2)
#define F4 NOTE(65,2)
#define G4 NOTE(67,2)
#define A4 NOTE(69,2)

#define C2 NOTE(60,4)
#define E2 NOTE(64,4)

void frog_round(sink_type sink, int wait)
{
    int msec = wait*8*60*1000/tempo;

    thread_ex::utc_time utc;
    utc += thread_ex::seconds(msec / 1000);
    utc += thread_ex::milliseconds(msec % 1000);
    boost::thread::sleep(utc);

    const audio::pcm_format& fmt = sink.format();

    using namespace io_ex::cat_operators;

    // L8
    io::copy(
        C4 + D4 + E4 + F4 + E4 + D4 + C2 +
        E4 + F4 + G4 + A4 + G4 + F4 + E2 +
        C2      + C2      + C2      + C2 +
        CC + DD + EE + FF + E4 + D4 + C2 ,
        audio::widen<float>(sink)
    );
}

int main()
{
    try
    {
        audio::pcm_format fmt;
        fmt.type = audio::uint8;
        fmt.channels = 2;
        fmt.rate = 22050;

#if defined(BOOST_WINDOWS)
        audio::direct_sound_device dev;
        dev.set_cooperative_level(0, audio::direct_sound::priority_level);
        dev.format(fmt);
#endif

        boost::thread_group threads;
        for (int i = 0; i < 3; ++i)
        {
#if defined(BOOST_WINDOWS)
            sink_type sink =
                dev.create_buffer(fmt, fmt.optimal_buffer_size());
#else
            sink_type sink(
                "frog_round",
                ("Part " + hamigaki::to_dec<char>(i)).c_str(),
                fmt
            );
#endif
            threads.create_thread(boost::bind(&frog_round, sink, i));
        }
        threads.join_all();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
