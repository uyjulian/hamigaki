// map_edit_window.hpp: the window for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAP_WINDOW_HPP
#define MAP_WINDOW_HPP

#include <string>
#include <windows.h>

::ATOM register_map_edit_window_class(::HINSTANCE hInstance);

::HWND create_map_edit_window(
    ::HWND parent, int id, int left, ::HINSTANCE hInstance, ::ATOM cls);

void map_edit_window_load(::HWND hwnd, const std::string& filename);
void map_edit_window_save(::HWND hwnd, const std::string& filename);
void map_edit_window_select_char(::HWND hwnd, char c);

#endif // MAP_WINDOW_HPP
