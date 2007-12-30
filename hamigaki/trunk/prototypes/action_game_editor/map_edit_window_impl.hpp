// map_edit_window_impl.hpp: the window implementation for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAP_EDIT_WINDOW_IMPL_HPP
#define MAP_EDIT_WINDOW_IMPL_HPP

#include "game_character_class.hpp"
#include "stage_map.hpp"
#include <hamigaki/uuid.hpp>
#include <boost/shared_ptr.hpp>
#include <set>
#include <string>
#include <windows.h>

class map_edit_window
{
public:
    explicit map_edit_window(::HWND handle);
    ~map_edit_window();
    void set_bg_color(unsigned long color);
    void set_characters(std::set<game_character_class>* chars);
    void set_stage(stage_map* map);
    void render();
    void reset_d3d();
    void update_scroll_box();
    void horz_scroll_pos(int pos);
    void vert_scroll_pos(int pos);
    void cursor_pos(int x, int y);
    void select_char(const hamigaki::uuid& c);
    void put_char();
    bool modified() const;
    void mouse_captured(bool value);
    bool mouse_captured() const;

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // MAP_EDIT_WINDOW_IMPL_HPP
