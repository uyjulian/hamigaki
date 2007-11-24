// map_edit_window_impl.hpp: the window implementation for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAP_EDIT_WINDOW_IMPL_HPP
#define MAP_EDIT_WINDOW_IMPL_HPP

#include <boost/shared_ptr.hpp>
#include <windows.h>

class map_edit_window
{
public:
    explicit map_edit_window(::HWND handle);
    ~map_edit_window();
    void render();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // MAP_EDIT_WINDOW_IMPL_HPP
