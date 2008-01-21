// icon_view_window_impl.hpp: the window implementation for icon view

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ICON_VIEW_WINDOW_IMPL_HPP
#define ICON_VIEW_WINDOW_IMPL_HPP

#include "physics_types.hpp"
#include <boost/shared_ptr.hpp>
#include <string>
#include <utility>
#include <windows.h>

class icon_view_window
{
public:
    explicit icon_view_window(::HWND handle);
    ~icon_view_window();
    void render();
    void load(const std::string& filename, const rectangle<int>& r);
    std::string filename() const;
    rectangle<int> icon_rect() const;

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // ICON_VIEW_WINDOW_IMPL_HPP
