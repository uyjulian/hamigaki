// main_window.hpp: main window for sound_player

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include "audio_info.hpp"
#include <windows.h>

class main_window
{
public:
    explicit main_window(::HWND handle);

    void open(const std::string& filename);
    void play();
    void stop();
    void pause();
    bool playing() const;
    audio_info info() const;

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // MAIN_WINDOW_HPP
