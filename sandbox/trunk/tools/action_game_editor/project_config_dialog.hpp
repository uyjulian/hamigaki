// project_config_dialog.hpp: the dialog to input data for setting a project

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PROJECT_SETTING_DIALOG_HPP
#define PROJECT_SETTING_DIALOG_HPP

#include "game_project.hpp"
#include <string>
#include <vector>
#include <windows.h>

bool get_project_info(
    ::HWND hwnd, game_project& info, const std::vector<std::string>& map_names);

bool get_project_info(::HWND hwnd, game_project& info);

#endif // PROJECT_SETTING_DIALOG_HPP
