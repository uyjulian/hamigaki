// position_dialog.hpp: the dialog to input data for setting a project

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef POSITION_DIALOG_HPP
#define POSITION_DIALOG_HPP

#include "game_character_class.hpp"
#include "stage_map.hpp"
#include <map>
#include <set>
#include <string>
#include <windows.h>

struct map_position_info
{
    std::map<std::string,stage_map>* map_table;
    std::set<game_character_class>* chars;
    unsigned long bg_color;

    std::string filename;
    int x;
    int y;
};

bool get_map_position(::HWND hwnd, map_position_info& info);

#endif // POSITION_DIALOG_HPP
