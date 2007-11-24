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
    explicit main_window(::HWND handle);
    ~main_window();
    void update_size();
    void load_stage(const std::string& filename);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // MAIN_WINDOW_IMPL_HPP
