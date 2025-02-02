// char_select_window_impl.hpp: the window implementation for character select

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef CHAR_SELECT_WINDOW_IMPL_HPP
#define CHAR_SELECT_WINDOW_IMPL_HPP

#include "game_character_class.hpp"
#include <hamigaki/uuid.hpp>
#include <boost/shared_ptr.hpp>
#include <set>
#include <string>
#include <windows.h>

class char_select_window
{
public:
    explicit char_select_window(::HWND handle);
    ~char_select_window();
    void set_bg_color(unsigned long color);
    void set_characters(std::set<game_character_class>* chars);
    void render();
    void cursor_pos(int x, int y);
    game_character_class* selected_char() const;
    hamigaki::uuid selected_char_id() const;
    bool modified() const;
    void modified(bool value);
    bool insert(const game_character_class& c);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // CHAR_SELECT_WINDOW_IMPL_HPP
