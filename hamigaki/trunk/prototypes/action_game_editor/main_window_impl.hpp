// main_window.hpp: main window implementation for action_game_editor

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAIN_WINDOW_IMPL_HPP
#define MAIN_WINDOW_IMPL_HPP

#include "game_project.hpp"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <windows.h>

class main_window
{
public:
    static const int char_select_id = 1;
    static const int map_edit_id = 2;
    static const int map_select_id = 3;

    explicit main_window(::HWND handle);
    ~main_window();
    void update_size();
    void update_selected_char();
    void edit_additional_data();
    void new_project(const std::string& filename, const game_project& proj);
    void close_project();
    void load_project(const std::string& filename);
    void save_project();
    game_project project_info() const;
    void project_info(const game_project& info);
    bool new_stage(const std::string& filename, int width, int height);
    int stage_count() const;
    std::string stage_name() const;
    void get_stage_names(std::vector<std::string>& names) const;
    void delete_stage();
    void change_stage();
    bool modified();
    void track_popup_menu(::HWND hwnd, int x, int y);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // MAIN_WINDOW_IMPL_HPP
