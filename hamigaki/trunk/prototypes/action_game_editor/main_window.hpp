// main_window.hpp: main window for action_game_editor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <string>
#include <windows.h>

::ATOM register_main_window_class(::HINSTANCE hInstance);
::HWND create_main_window(::HINSTANCE hInstance);

#endif // MAIN_WINDOW_HPP
