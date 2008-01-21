// transfer_dialog.hpp: the dialog to input data for setting a transfer

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef TRANSFER_DIALOG_HPP
#define TRANSFER_DIALOG_HPP

#include "game_character_class.hpp"
#include "stage_map.hpp"
#include "transfer_info.hpp"
#include <map>
#include <set>
#include <string>
#include <windows.h>

struct transfer_info_params
{
    std::map<std::string,stage_map>* map_table;
    std::set<game_character_class>* chars;
    unsigned long bg_color;

    transfer_info info;
};

bool get_transfer_info(::HWND hwnd, transfer_info_params& params);

#endif // TRANSFER_DIALOG_HPP
