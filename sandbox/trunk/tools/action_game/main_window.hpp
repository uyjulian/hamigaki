// main_window_class.hpp: main window for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "game_project.hpp"
#include <string>
#include <windows.h>

::ATOM register_main_window_class(::HINSTANCE hInstance);

::HWND create_main_window(
    ::HINSTANCE hInstance, ::ATOM cls, const game_project& proj);

void connect_d3d_device(::HWND hwnd);
bool process_input(::HWND hwnd);
void render(::HWND hwnd);

#endif // MAIN_WINDOW_HPP
