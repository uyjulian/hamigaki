// char_class_dialog.hpp: the dialog to input data for editing characters

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef NEW_STAGE_DIALOG_HPP
#define NEW_STAGE_DIALOG_HPP

#include "game_character_class.hpp"
#include <windows.h>

bool get_character_class_info(::HWND hwnd, game_character_class& info);

#endif // NEW_STAGE_DIALOG_HPP
