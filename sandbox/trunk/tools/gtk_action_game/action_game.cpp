// action_game.cpp: an action game of the prototype

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "game_project_io.hpp"
#include "main_window.hpp"
#include <hamigaki/system/windows_error.hpp>
#include <boost/filesystem/path.hpp>
#include <exception>
#include <iostream>

#include <unistd.h>

namespace fs = boost::filesystem;
using hamigaki::system::windows_error;

int main(int argc, char* argv[])
{
    try
    {
        ::gtk_set_locale();
        ::gtk_init(&argc, &argv);

        const char* filename = (argc >= 2) ? argv[1] : "action_game.agp-yh";
        game_project proj = load_game_project(filename);

        fs::path dir = fs::path(filename).branch_path();
        if (!dir.empty())
            ::chdir(dir.directory_string().c_str());

        GtkWidget* window = ::gtk_window_new(GTK_WINDOW_TOPLEVEL);
        if (window == 0)
            throw std::runtime_error("gtk_window_new() failed");

        main_window_data* pimpl = 0;
        connect_signal(window, "destroy", G_CALLBACK(destroy), &pimpl);
        connect_signal(window, "realize", G_CALLBACK(realize), &pimpl);
        connect_signal(window, "unrealize", G_CALLBACK(unrealize), &pimpl);
        ::g_timeout_add(16, &render, &pimpl);

        ::gtk_widget_show_all(window);
        ::gtk_main();

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
