//  main_dialog.hpp: main dialog for sound_player

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef MAIN_DIALOG_HPP
#define MAIN_DIALOG_HPP

#include <win32gui/window.hpp>
#include <hamigaki/audio/background_player.hpp>
#include <string>

class main_dialog :
    public win32::gui::wnd_extend<win32::gui::dialog,main_dialog>
{
public:
    main_dialog();
    ~main_dialog();

    static int dialog_id();

    void open(const std::string& filename);
    void play();
    void stop();
    void pause();
    void toggle_play();
    void update_progress();

private:
    hamigaki::audio::background_player player_;
    int block_size_;
};

#endif // MAIN_DIALOG_HPP
