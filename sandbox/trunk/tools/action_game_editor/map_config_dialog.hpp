// map_config_dialog.hpp: the dialog to input data for setting a map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAP_SETTING_DIALOG_HPP
#define MAP_SETTING_DIALOG_HPP

#include <string>
#include <windows.h>

struct stage_info
{
    std::string name;
    int width;
    int height;

    stage_info() : width(200), height(15)
    {
    }
};

bool get_stage_info(::HWND hwnd, stage_info& info);

#endif // MAP_SETTING_DIALOG_HPP
