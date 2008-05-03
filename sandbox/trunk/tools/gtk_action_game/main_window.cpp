// main_window_class.cpp: main window for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "main_window.hpp"
#include <iostream>
#include <stdexcept>

namespace
{

void destroy(GtkWidget*, gpointer)
{
    ::gtk_main_quit();
}

void realize(GtkWidget* widget, gpointer user_data)
{
    main_window_data& data = *static_cast<main_window_data*>(user_data);
    try
    {
        data.pimpl = new main_window(widget, data.proj);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        data.pimpl = 0;
    }
}

void unrealize(GtkWidget*, gpointer user_data)
{
    main_window*& pimpl = *static_cast<main_window**>(user_data);
    delete pimpl;
    pimpl = 0;
}

gboolean frame(gpointer user_data)
{
    if (main_window*& pimpl = *static_cast<main_window**>(user_data))
    {
        try
        {
            if (pimpl->process_input())
                pimpl->render();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return TRUE;
}

template<class Widget, class Arg>
inline gulong connect_signal(
    Widget* w, const char* sig, GCallback func, Arg* arg)
{
    gulong id = ::g_signal_connect(G_OBJECT(w), sig, func, arg);
    if (id == 0)
        throw std::runtime_error("g_signal_connect() failed");
    return id;
}

} // namespace

GtkWidget* create_main_window(main_window_data& data)
{
    GtkWidget* window = ::gtk_window_new(GTK_WINDOW_TOPLEVEL);
    if (window == 0)
        throw std::runtime_error("gtk_window_new() failed");

    ::gtk_window_set_default_size(
        GTK_WINDOW(window), data.proj.screen_width, data.proj.screen_height);

    connect_signal(window, "destroy", G_CALLBACK(destroy), &data.pimpl);
    connect_signal(window, "realize", G_CALLBACK(realize), &data);
    connect_signal(window, "unrealize", G_CALLBACK(unrealize), &data.pimpl);
    ::g_timeout_add(16, &frame, &data.pimpl);

    return window;
}
