// sound_engine.hpp: sound engine for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SOUND_ENGINE_HPP
#define SOUND_ENGINE_HPP

#include <boost/shared_ptr.hpp>
#include <string>

class sound_engine
{
public:
    explicit sound_engine(void* hwnd);
    ~sound_engine();

    void play_bgm(const std::string& filename);
    void stop_bgm();
    void play_se(const std::string& filename);
    void stop_se();
    std::string se_filename() const;
    bool playing_se();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // SOUND_ENGINE_HPP
