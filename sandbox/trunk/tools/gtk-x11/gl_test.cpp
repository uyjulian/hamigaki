// gl_test.cpp: OpenGL test program for GTK+2/X11

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <cassert>
#include <stdexcept>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glx.h>

template<class T>
class scoped_x_ptr
{
public:
    explicit scoped_x_ptr(T* p = 0) : ptr_(p)
    {
    }

    ~scoped_x_ptr()
    {
        if (ptr_ != 0)
            ::XFree(ptr_);
    }

    T* get() const
    {
        return ptr_;
    }

private:
    T* ptr_;

    scoped_x_ptr(const scoped_x_ptr&);
    scoped_x_ptr& operator=(const scoped_x_ptr&);
};

class render_context
{
public:
    explicit render_context(::GtkWindow* window)
        : window_(window), handle_(0)
    {
        ::Window win = window_id();

        ::GdkScreen* scr = ::gtk_window_get_screen(window_);
        ::Display* dpy = GDK_SCREEN_XDISPLAY(scr);
        int scr_num = ::gdk_screen_get_number(scr);

        int attrs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, 0 };
        scoped_x_ptr< ::XVisualInfo> vi(::glXChooseVisual(dpy, scr_num, attrs));
        if (vi.get() == 0)
            throw std::runtime_error("glXChooseVisual() failed");

        handle_ = ::glXCreateContext(dpy, vi.get(), 0, True);
        if (handle_ == 0)
            throw std::runtime_error("glXCreateContext() failed");
        ::glXMakeCurrent(dpy, win, handle_);
    }

    ~render_context()
    {
        ::glXDestroyContext(display(), handle_);
    }

    void select()
    {
        ::glXMakeCurrent(display(), window_id(), handle_);
    }

    void swap_buffers()
    {
        assert(::glXGetCurrentContext() == handle_);

        ::glXSwapBuffers(display(), window_id());
    }

private:
    ::GtkWindow* window_;
    ::GLXContext handle_;

    ::Window window_id() const
    {
        return GDK_WINDOW_XID(GTK_WIDGET(window_)->window);
    }

    ::Display* display() const
    {
        return GDK_SCREEN_XDISPLAY(::gtk_window_get_screen(window_));
    }

    render_context(const render_context&);
    render_context& operator=(const render_context&);
};

class texture
{
public:
    explicit texture(render_context& rc) : handle_(0)
    {
        rc.select();
        ::glGenTextures(1, &handle_);
        if (::glGetError() != GL_NO_ERROR)
            throw std::runtime_error("glGenTextures() failed");
    }

    ~texture()
    {
        ::glDeleteTextures(1, &handle_);
    }

    void set_image(int width, int height, void* data)
    {
        ::glBindTexture(GL_TEXTURE_2D, handle_);
        ::glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        ::glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data);
        ::glBindTexture(GL_TEXTURE_2D, 0);
    }

    void bind()
    {
        ::glBindTexture(GL_TEXTURE_2D, handle_);
    }

private:
    ::GLuint handle_;

    texture(const texture&);
    texture& operator=(const texture&);
};

class main_window_data
{
public:
    explicit main_window_data(::GtkWindow* window) : rc_(window), texture_(rc_)
    {
        rc_.select();

        unsigned char r_table[] = { 0xFF, 0x00, 0x00, 0xFF };
        unsigned char g_table[] = { 0x00, 0xFF, 0x00, 0xFF };
        unsigned char b_table[] = { 0x00, 0x00, 0xFF, 0xFF };
        unsigned char a_table[] = { 0xFF, 0xFF, 0xFF, 0xFF };

        unsigned char data[4*64*64];
        unsigned offset = 0;
        for (unsigned y = 0; y < 64; ++y)
        {
            for (unsigned x = 0; x < 64; ++x)
            {
                unsigned type =
                    static_cast<unsigned>(x >= 32) +
                    (static_cast<unsigned>(y >= 32) << 1);

                data[offset++] = r_table[type];
                data[offset++] = g_table[type];
                data[offset++] = b_table[type];
                data[offset++] = a_table[type];
            }
        }
        texture_.set_image(64, 64, data);
    }

    void render()
    {
        rc_.select();

        ::glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        ::glClear(GL_COLOR_BUFFER_BIT);

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

    void resize(int width, int height)
    {
        rc_.select();
        ::glViewport(0, 0, width, height);
    }

private:
    render_context rc_;
    texture texture_;

    main_window_data(const main_window_data&);
    main_window_data& operator=(const main_window_data&);
};


#include <iostream>

void destroy(::GtkWidget*, ::gpointer)
{
    ::gtk_main_quit();
}

void realize(::GtkWidget* widget, ::gpointer user_data)
{
    main_window_data*& pimpl = *static_cast<main_window_data**>(user_data);
    try
    {
        pimpl = new main_window_data(GTK_WINDOW(widget));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        pimpl = 0;
    }
}

void unrealize(::GtkWidget*, ::gpointer user_data)
{
    main_window_data*& pimpl = *static_cast<main_window_data**>(user_data);
    delete pimpl;
    pimpl = 0;
}

gboolean key_press(
    ::GtkWidget* widget, ::GdkEventKey* event, ::gpointer user_data)
{
    std::cout << "+ " << event->hardware_keycode << std::endl;
    return TRUE;
}

gboolean key_release(
    ::GtkWidget* widget, ::GdkEventKey* event, ::gpointer user_data)
{
    std::cout << "- " << event->hardware_keycode << std::endl;
    return TRUE;
}

::gboolean
expose(::GtkWidget* widget, ::GdkEventExpose* event, ::gpointer user_data)
{
    if (main_window_data*& pimpl = *static_cast<main_window_data**>(user_data))
    {
        ::gint width;
        ::gint height;
        gtk_window_get_size(GTK_WINDOW(widget), &width, &height);
        pimpl->resize(width, height);
    }

    return TRUE;
}

::gboolean draw(::gpointer user_data)
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
inline ::gulong connect_signal(
    Widget* w, const char* sig, ::GCallback func, Arg* arg)
{
    ::gulong id = ::g_signal_connect(G_OBJECT(w), sig, func, arg);
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

        ::GtkWidget* window = ::gtk_window_new(GTK_WINDOW_TOPLEVEL);
        if (window == 0)
            throw std::runtime_error("gtk_window_new() failed");

        ::gtk_window_set_default_size(GTK_WINDOW(window), 320, 240);

        main_window_data* pimpl = 0;
        connect_signal(window, "destroy", G_CALLBACK(destroy), &pimpl);
        connect_signal(window, "realize", G_CALLBACK(realize), &pimpl);
        connect_signal(window, "unrealize", G_CALLBACK(unrealize), &pimpl);

        connect_signal(
            window, "key-press-event", G_CALLBACK(key_press), &pimpl);
        connect_signal(
            window, "key-release-event", G_CALLBACK(key_release), &pimpl);

        connect_signal(window, "expose-event", G_CALLBACK(expose), &pimpl);

        ::g_idle_add(&draw, &pimpl);

        ::gtk_widget_show_all(window);

        ::GdkWMDecoration decs =
            static_cast< ::GdkWMDecoration>(
                GDK_DECOR_BORDER | GDK_DECOR_TITLE |
                GDK_DECOR_MENU | GDK_DECOR_MINIMIZE
            );
        ::gdk_window_set_decorations(window->window, decs);

        ::gtk_main();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
