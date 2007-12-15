// icon_view_window.hpp: the window for icon view

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ICON_VIEW_WINDOW_HPP
#define ICON_VIEW_WINDOW_HPP

#include "physics_types.hpp"
#include <string>
#include <utility>
#include <windows.h>

::ATOM register_icon_view_window_class(::HINSTANCE hInstance);
void icon_window_load(
    ::HWND hwnd, const std::string& filename, const rectangle<int>& r);

#endif // ICON_VIEW_WINDOW_HPP
