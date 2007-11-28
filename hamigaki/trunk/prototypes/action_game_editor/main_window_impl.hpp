// main_window.hpp: main window implementation for action_game_editor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAIN_WINDOW_IMPL_HPP
#define MAIN_WINDOW_IMPL_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include <windows.h>

class main_window
{
public:
    static const int char_select_id = 1;
    static const int map_edit_id = 2;

    explicit main_window(::HWND handle);
    ~main_window();
    void update_size();
    void update_selected_char();
    void new_stage(int width, int height);
    void load_stage(const std::string& filename);
    void save_stage(const std::string& filename);
    bool save_stage();
    bool modified();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // MAIN_WINDOW_IMPL_HPP
