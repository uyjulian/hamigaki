// sound_engine.cpp: sound engine for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "sound_engine.hpp"
#include <hamigaki/audio/vorbis/comment.hpp>
#include <hamigaki/audio/background_player.hpp>
#include <hamigaki/audio/direct_sound.hpp>
#include <hamigaki/audio/vorbis_file.hpp>
#include <hamigaki/audio/wide_adaptor.hpp>
#include <hamigaki/iostreams/background_copy.hpp>
#include <hamigaki/iostreams/concatenate.hpp>
#include <hamigaki/iostreams/lazy_restrict.hpp>
#include <hamigaki/iostreams/repeat.hpp>
#include <boost/lexical_cast.hpp>
#include <memory>

namespace audio = hamigaki::audio;
namespace ds = audio::direct_sound;
namespace io_ex = hamigaki::iostreams;

namespace
{

int vorbis_comment_int_value(
    const std::pair<const char**,const char**>& comments,
    const std::string& name)
{
    std::string value(audio::vorbis::comment_value(comments, name));
    return value.empty() ? -1 : boost::lexical_cast<int>(value);
}

} // namespace

class sound_engine::impl
{
public:
    explicit impl(void* handle)
    {
        dsound_.set_cooperative_level(handle, ds::priority_level);

        audio::pcm_format fmt;
        fmt.type = audio::int_le16;
        fmt.channels = 2;
        fmt.rate = 44100;

        dsound_.format(fmt);
    }

    void play_bgm(const std::string& filename)
    {
        if (bgm_player_)
            bgm_player_.close();
        bgm_file_.reset(new audio::vorbis_file_source(filename));

        const audio::vorbis_info& info = bgm_file_->info();
        const int block_size = info.channels;

        int loop_start =
            vorbis_comment_int_value(bgm_file_->comments(), "LOOPSTART");
        std::streamsize offset = loop_start != -1 ? block_size*loop_start : -1;

        int loop_length =
            vorbis_comment_int_value(bgm_file_->comments(), "LOOPLENGTH");
        std::streamsize len = loop_length != -1 ? block_size*loop_length : -1;

        audio::pcm_format fmt;
        fmt.type = audio::int_le16;
        fmt.channels = info.channels;
        fmt.rate = info.rate;

        if (offset == -1)
        {
            bgm_player_.open(
                io_ex::repeat(*bgm_file_, -1),
                audio::widen<float>(dsound_.create_buffer(fmt, 4096)),
                2048
            );
        }
        else
        {
            using namespace io_ex::cat_operators;

            bgm_player_.open(
                io_ex::lazy_restrict(*bgm_file_, 0, offset+len) +
                io_ex::lazy_restrict(*bgm_file_, offset, len) * -1,
                audio::widen<float>(dsound_.create_buffer(fmt, 4096)),
                2048
            );
        }
        bgm_player_.play();
    }

    void play_se(const std::string& filename)
    {
        se_player_.reset();
        se_file_.reset(new audio::vorbis_file_source(filename));

        const audio::vorbis_info& info = se_file_->info();

        audio::pcm_format fmt;
        fmt.type = audio::int_le16;
        fmt.channels = info.channels;
        fmt.rate = info.rate;

        se_player_.reset(
            new io_ex::background_copy(
                *se_file_,
                audio::widen<float>(dsound_.create_buffer(fmt, 1024)),
                512
            )
        );
    }

private:
    audio::direct_sound_device dsound_;
    std::auto_ptr<audio::vorbis_file_source> bgm_file_;
    std::auto_ptr<audio::vorbis_file_source> se_file_;
    audio::background_player bgm_player_;
    std::auto_ptr<io_ex::background_copy> se_player_;
};

sound_engine::sound_engine(void* handle) : pimpl_(new impl(handle))
{
}

sound_engine::~sound_engine()
{
}

void sound_engine::play_bgm(const std::string& filename)
{
    pimpl_->play_bgm(filename);
}

void sound_engine::play_se(const std::string& filename)
{
    pimpl_->play_se(filename);
}
