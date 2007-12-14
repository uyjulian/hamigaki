// icon_select_dialog.hpp: the dialog to select an icon

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ICON_SELECT_DIALOG_HPP
#define ICON_SELECT_DIALOG_HPP

#include <string>
#include <windows.h>

struct icon_info
{
    std::string filename;
    int x;
    int y;

    icon_info() : x(0), y(0)
    {
    }
};

bool select_icon(::HWND hwnd, icon_info& info);

#endif // ICON_SELECT_DIALOG_HPP
