// falcom_play.cpp: an ogg player for Falcom games (YS6,ED6,YSF)

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/pcm_device.hpp>
#include <hamigaki/audio/vorbis/comment.hpp>
#include <hamigaki/audio/vorbis_file.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <hamigaki/iostreams/concatenate.hpp>
#include <hamigaki/iostreams/lazy_restrict.hpp>
#include <hamigaki/iostreams/repeat.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/restrict.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include <iostream>

namespace audio = hamigaki::audio;
namespace io = boost::iostreams;
namespace io_ex = hamigaki::iostreams;

int vorbis_comment_int_value(
    const std::pair<const char**,const char**>& comments,
    const std::string& name)
{
    std::string value(audio::vorbis::comment_value(comments, name));
    return value.empty() ? -1 : boost::lexical_cast<int>(value);
}

int main(int argc, char* argv[])
{
    try
    {
        if ((argc != 2) && (argc != 3) && (argc != 4))
        {
            std::cerr
                << "Usage: falcom_play (input file) [loop count] [epilogue]\n"
                << "Options:\n"
                << "  loop count    -1 -> infinity, 0 -> no loop (default=-1)\n"
                << "  epilogue      play the epilogue (0 or 1, default=1)\n"
                << std::flush;

            return 1;
        }

        int loop_count = (argc >= 3) ? boost::lexical_cast<int>(argv[2]) : -1;
        bool play_epilogue =
            (argc == 4) ? boost::lexical_cast<int>(argv[3]) != 0 : true;

        audio::vorbis_file_source vf(argv[1]);

        const audio::vorbis_info info = vf.info();

        audio::pcm_format fmt;
        fmt.type = audio::int_le16;
        fmt.channels = info.channels;
        fmt.rate = info.rate;
        const int block_size = 2;

        int loop_start =
            vorbis_comment_int_value(vf.comments(), "LOOPSTART");
        std::streamsize offset = loop_start != -1 ? block_size*loop_start : -1;

        int loop_length =
            vorbis_comment_int_value(vf.comments(), "LOOPLENGTH");
        std::streamsize len = loop_length != -1 ? block_size*loop_length : -1;
        std::streamsize epi_len = play_epilogue ? -1 : 0;

        audio::pcm_sink pcm(fmt);
        if (offset == -1)
            io::copy(vf, audio::widen<float>(pcm), vf.optimal_buffer_size());
        else
        {
            using namespace io_ex::cat_operators;
            io::copy(
                io_ex::lazy_restrict(vf, 0, offset+len)
                + io_ex::lazy_restrict(vf, offset, len) * loop_count
                + io_ex::lazy_restrict(vf, offset+len, epi_len),
                audio::widen<float>(pcm),
                vf.optimal_buffer_size()
            );
        }
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
