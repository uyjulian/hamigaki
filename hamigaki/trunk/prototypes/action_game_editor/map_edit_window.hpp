// map_edit_window.hpp: the window for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAP_WINDOW_HPP
#define MAP_WINDOW_HPP

#include "game_character_class.hpp"
#include "stage_map.hpp"
#include <hamigaki/uuid.hpp>
#include <set>
#include <string>
#include <windows.h>

::ATOM register_map_edit_window_class(::HINSTANCE hInstance);

::HWND create_map_edit_window(
    ::HWND parent, int id, int left, ::HINSTANCE hInstance);

void map_edit_window_set(::HWND hwnd, stage_map* map);

void map_edit_window_set_char_list(
    ::HWND hwnd, std::set<game_character_class>* chars);

void map_edit_window_select_char(::HWND hwnd, const hamigaki::uuid& c);
bool map_edit_window_modified(::HWND hwnd);

#endif // MAP_WINDOW_HPP
