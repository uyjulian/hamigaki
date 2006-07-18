//  frog_round.cpp: an example for DirectSound multi-buffer mixing

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hamigaki/audio/amplify.hpp>
#include <hamigaki/audio/direct_sound.hpp>
#include <hamigaki/audio/square_wave.hpp>
#include <hamigaki/audio/stereo.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <hamigaki/iostreams/catable/restrict.hpp>
#include <hamigaki/iostreams/tiny_restrict.hpp>
#include <hamigaki/thread/utc_time.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include "../test/detail/utility.hpp"

namespace audio = hamigaki::audio;
namespace ds = audio::direct_sound;
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

void frog_round(audio::direct_sound_buffer buffer, int wait)
{
    int msec = wait*8*60*1000/tempo;

    thread_ex::utc_time utc;
    utc += thread_ex::seconds(msec / 1000);
    utc += thread_ex::milliseconds(msec % 1000);
    boost::thread::sleep(utc);

    const audio::pcm_format& fmt = buffer.format();

    using namespace io_ex::cat_operators;

    // L8
    io::copy(
        C4 + D4 + E4 + F4 + E4 + D4 + C2 +
        E4 + F4 + G4 + A4 + G4 + F4 + E2 +
        C2      + C2      + C2      + C2 +
        CC + DD + EE + FF + E4 + D4 + C2 ,
        audio::widen<float>(buffer)
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

        audio::direct_sound_device dev;
        dev.set_cooperative_level(0, ds::priority_level);
        dev.format(fmt);

        boost::thread_group threads;
        for (int i = 0; i < 3; ++i)
        {
            audio::direct_sound_buffer buffer =
                dev.create_buffer(fmt, fmt.optimal_buffer_size());
            threads.create_thread(boost::bind(&frog_round, buffer, i));
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
