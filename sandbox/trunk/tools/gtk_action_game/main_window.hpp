// main_window_class.hpp: main window for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include "game_project.hpp"
#include <gtk/gtk.h>

class main_window;

struct main_window_data
{
    main_window_data() : pimpl(0)
    {
    }

    main_window* pimpl;
    game_project proj;
};

GtkWidget* create_main_window(main_window_data& data);

#endif // MAIN_WINDOW_HPP
