// png_test.cpp: the test program for the class png_reader

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <boost/config.hpp>

#include "png_reader.hpp"
#include "texture.hpp"
#include "render_context.hpp"
#include <boost/scoped_array.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#endif
#include <GL/gl.h>

class main_window_data
{
public:
    explicit main_window_data(GtkWidget* widget) : rc_(widget), texture_(rc_)
    {
        rc_.select();

        std::ifstream is("milk.png", std::ios_base::binary);

        hamigaki::png_reader png(is);
        const int width = static_cast<int>(png.width());
        const int height = static_cast<int>(png.height());
        const unsigned long pitch = png.pitch();

        typedef unsigned char ubyte;
        boost::scoped_array<ubyte> row(new ubyte[pitch]);
        boost::scoped_array<ubyte> data(new ubyte[4*width*height]);

        const std::size_t data_pitch = static_cast<std::size_t>(4*width);
        for (int i = 0; i < height; ++i)
        {
            png.read_next_row(row.get());
            std::memcpy(&data[data_pitch*i], row.get(), data_pitch);
        }

        texture_.set_image(width, height, data.get());
    }

    void render()
    {
        rc_.select();

        ::glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT);

        ::glAlphaFunc(GL_GREATER, 0.1f);
        ::glEnable(GL_ALPHA_TEST);
        ::glEnable(GL_TEXTURE_2D);
        texture_.bind();

        ::glBegin(GL_QUADS);
        ::glTexCoord2f(0.0f, 0.0f);
        ::glVertex3f(-0.75f, 0.75f, 0.0f);
        ::glTexCoord2f(1.0f, 0.0f);
        ::glVertex3f(0.75f, 0.75f, 0.0f);
        ::glTexCoord2f(1.0f, 1.0f);
        ::glVertex3f(0.75f, -0.75f, 0.0f);
        ::glTexCoord2f(0.0f, 1.0f);
        ::glVertex3f(-0.75f, -0.75f, 0.0f);
        ::glEnd();

        ::glDisable(GL_TEXTURE_2D);

        rc_.swap_buffers();
    }

private:
    hamigaki::render_context rc_;
    hamigaki::texture texture_;

    main_window_data(const main_window_data&);
    main_window_data& operator=(const main_window_data&);
};

void destroy(GtkWidget*, gpointer)
{
    ::gtk_main_quit();
}

void realize(GtkWidget* widget, gpointer user_data)
{
    main_window_data*& pimpl = *static_cast<main_window_data**>(user_data);
    try
    {
        pimpl = new main_window_data(widget);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        pimpl = 0;
    }
}

void unrealize(GtkWidget*, gpointer user_data)
{
    main_window_data*& pimpl = *static_cast<main_window_data**>(user_data);
    delete pimpl;
    pimpl = 0;
}

gboolean render(gpointer user_data)
{
    if (main_window_data*& pimpl = *static_cast<main_window_data**>(user_data))
    {
        try
        {
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

int main(int argc, char* argv[])
{
    try
    {
        ::gtk_set_locale();
        ::gtk_init(&argc, &argv);

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
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
