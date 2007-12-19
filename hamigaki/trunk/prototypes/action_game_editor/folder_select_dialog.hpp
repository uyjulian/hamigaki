// folder_select_dialog.hpp: the dialog to select an folder

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ICON_SELECT_DIALOG_HPP
#define ICON_SELECT_DIALOG_HPP

#include <string>
#include <windows.h>

bool select_folder(::HWND hwnd, std::string& path);

#endif // ICON_SELECT_DIALOG_HPP
