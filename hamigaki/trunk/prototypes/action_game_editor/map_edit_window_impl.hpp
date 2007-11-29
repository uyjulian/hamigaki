// map_edit_window_impl.hpp: the window implementation for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAP_EDIT_WINDOW_IMPL_HPP
#define MAP_EDIT_WINDOW_IMPL_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include <windows.h>

class map_edit_window
{
public:
    explicit map_edit_window(::HWND handle);
    ~map_edit_window();
    void new_stage(int width, int height);
    void load_stage(const std::string& filename);
    void save_stage(const std::string& filename);
    void render();
    void reset_d3d();
    void update_scroll_box();
    void horz_scroll_pos(int pos);
    void vert_scroll_pos(int pos);
    void cursor_pos(int x, int y);
    void select_char(char c);
    void put_char();
    bool modified() const;
    void mouse_captured(bool value);
    bool mouse_captured() const;

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // MAP_EDIT_WINDOW_IMPL_HPP
