//  main_window_class.hpp: main window for bjam_win

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <windows.h>

::ATOM register_main_window_class(::HINSTANCE hInstance);
::HWND create_main_window(::HINSTANCE hInstance, ::ATOM cls);

#endif // MAIN_WINDOW_HPP
