//  sound_player.cpp: a GUI example for background_player

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include "main_dialog.hpp"
#include <win32gui/frame.hpp>
#include "win32gui_res/menus.hpp"

namespace gui = win32::gui;

int WINAPI WinMain(::HINSTANCE, ::HINSTANCE, ::LPSTR, int)
{
    try
    {
        gui::wnd<gui::sdi_frame> top = gui::create_wnd<gui::sdi_frame>(
            "Sound Player", gui::null_wnd,
            gui::create_info().menu(gui::res_id::menu_::menu));

        gui::rectangle wr = top->window_rect(gui::rel_to_screen);
        gui::rectangle cr = top->client_rect();
        top->move(gui::rectangle(
            wr.top_left(),
            gui::wnd_size(
                wr.width()-cr.width()+280,
                wr.height()-cr.height()+100)), true);

        gui::create_dlg<main_dialog>(top);
        top->wait();
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(0, e.what(), "Error", MB_OK);
    }
    return 0;
}
