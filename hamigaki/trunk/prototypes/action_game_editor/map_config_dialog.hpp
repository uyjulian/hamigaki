// new_stage_dialog.hpp: the dialog to input data for creating a new stage

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef NEW_STAGE_DIALOG_HPP
#define NEW_STAGE_DIALOG_HPP

#include <windows.h>

struct stage_info
{
    int width;
    int height;

    stage_info() : width(200), height(15)
    {
    }
};

bool get_new_stage_info(::HWND hwnd, stage_info& info);

#endif // NEW_STAGE_DIALOG_HPP
