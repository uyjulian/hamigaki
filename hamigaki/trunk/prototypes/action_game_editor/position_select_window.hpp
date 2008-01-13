// position_select_window.hpp: the window for position selection

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef POSITION_SELECT_WINDOW_HPP
#define POSITION_SELECT_WINDOW_HPP

#include "game_character_class.hpp"
#include "stage_map.hpp"
#include <set>
#include <windows.h>

::ATOM register_position_select_window_class(::HINSTANCE hInstance);

::HWND create_position_select_window(
    ::HWND parent, int id, int left, ::HINSTANCE hInstance);

void position_select_window_set_bg_color(::HWND hwnd, unsigned long color);
void position_select_window_set(::HWND hwnd, stage_map* map);

void position_select_window_set_char_list(
    ::HWND hwnd, std::set<game_character_class>* chars);

std::pair<int,int> position_select_window_selected_pos(::HWND hwnd);
void position_select_window_selected_pos(::HWND hwnd, int x, int y);

#endif // POSITION_SELECT_WINDOW_HPP
