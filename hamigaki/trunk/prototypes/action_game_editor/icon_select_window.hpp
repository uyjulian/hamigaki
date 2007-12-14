// icon_select_window.hpp: the window for icon selection

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ICON_SELECT_WINDOW_HPP
#define ICON_SELECT_WINDOW_HPP

#include <string>
#include <utility>
#include <windows.h>

::ATOM register_icon_select_window_class(::HINSTANCE hInstance);
void select_window_load(::HWND hwnd, const std::string& filename);
void select_window_cursor_pos(::HWND hwnd, int x, int y);
std::pair<int,int> select_window_cursor_pos(::HWND hwnd);

#endif // ICON_SELECT_WINDOW_HPP
