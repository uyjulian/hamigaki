// char_select_window.hpp: the window for character selection

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef CHAR_SELECT_WINDOW_HPP
#define CHAR_SELECT_WINDOW_HPP

#include "game_character_class.hpp"
#include <hamigaki/uuid.hpp>
#include <set>
#include <string>
#include <windows.h>

::ATOM register_char_select_window_class(::HINSTANCE hInstance);
::HWND create_char_select_window(::HWND parent, int id, ::HINSTANCE hInstance);
hamigaki::uuid get_selected_char(::HWND hwnd);
void char_select_window_set_bg_color(::HWND hwnd, unsigned long color);
void setup_char_list(::HWND hwnd, std::set<game_character_class>* chars);
bool char_select_window_modified(::HWND hwnd);
void char_select_window_modified(::HWND hwnd, bool value);

#endif // CHAR_SELECT_WINDOW_HPP
