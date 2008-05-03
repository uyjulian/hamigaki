// action_game.cpp: an action game of the prototype

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <boost/config.hpp>

#include "game_project_io.hpp"
#include "main_window.hpp"
#include <hamigaki/system/windows_error.hpp>
#include <boost/filesystem/path.hpp>
#include <exception>
#include <iostream>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#else
    #include <unistd.h>
#endif

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
        {
#if defined(BOOST_WINDOWS)
            ::SetCurrentDirectoryA(dir.directory_string().c_str());
#else
            ::chdir(dir.directory_string().c_str());
#endif
        }

        main_window_data data;
        data.proj = proj;
        GtkWidget* window = create_main_window(data);
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
