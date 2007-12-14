// icon_select_window_impl.hpp: the window implementation for icon select

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ICON_SELECT_WINDOW_IMPL_HPP
#define ICON_SELECT_WINDOW_IMPL_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include <utility>
#include <windows.h>

class icon_select_window
{
public:
    explicit icon_select_window(::HWND handle);
    ~icon_select_window();
    void render();
    void cursor_pos(int x, int y);
    std::pair<int,int> cursor_pos() const;
    void load(const std::string& filename);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // ICON_SELECT_WINDOW_IMPL_HPP
