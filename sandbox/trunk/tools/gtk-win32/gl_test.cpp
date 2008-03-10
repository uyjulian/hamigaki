// gl_test.cpp: OpenGL test program for GTK+2/Win32

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <cassert>
#include <stdexcept>
#include <gdk/gdkwin32.h>
#include <gtk/gtk.h>
#include <GL/gl.h>

class graphic_context
{
public:
    explicit graphic_context(::GdkWindow* window)
        : handle_(::gdk_gc_new(window))
    {
        if (handle_ == 0)
            throw std::runtime_error("gdk_gc_new() failed");
    }

    ~graphic_context()
    {
        ::gdk_gc_unref(handle_);
    }

    ::GdkGC* get() const
    {
        return handle_;
    }

private:
    ::GdkGC* handle_;

    graphic_context(const graphic_context&);
    graphic_context& operator=(const graphic_context&);
};

class device_context
{
public:
    explicit device_context(::GdkWindow* window)
        : window_(window), gc_(window), handle_(0)
    {
        handle_ = ::gdk_win32_hdc_get(window_, gc_.get(), ::GdkGCValuesMask());
        if (handle_ == 0)
            throw std::runtime_error("gdk_win32_hdc_get() failed");
    }

    ~device_context()
    {
        ::gdk_win32_hdc_release(window_, gc_.get(), ::GdkGCValuesMask());
    }

    ::HDC get() const
    {
        return handle_;
    }

private:
    ::GdkWindow* window_;
    graphic_context gc_;
    ::HDC handle_;

    device_context(const device_context&);
    device_context& operator=(const device_context&);
};

class render_context
{
public:
    explicit render_context(::GdkWindow* window) : dc_(window), handle_(0)
    {
        ::PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 16;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int fmt = ::ChoosePixelFormat(dc_.get(), &pfd);
        if (fmt == 0)
            throw std::runtime_error("ChoosePixelFormat() failed");
        ::SetPixelFormat(dc_.get(), fmt, &pfd);

        handle_ = ::wglCreateContext(dc_.get());
        if (handle_ == 0)
            throw std::runtime_error("wglCreateContext() failed");
    }

    ~render_context()
    {
        ::wglDeleteContext(handle_);
    }

    void select()
    {
        ::wglMakeCurrent(dc_.get(), handle_);
    }

    void swap_buffers()
    {
        assert(::wglGetCurrentContext() == handle_);

        ::SwapBuffers(dc_.get());
    }

    void clear(float r, float g, float b, float a)
    {
        assert(::wglGetCurrentContext() == handle_);

        ::glClearColor(r, g, b, a);
        ::glClear(GL_COLOR_BUFFER_BIT);
    }

private:
    device_context dc_;
    ::HGLRC handle_;

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
    explicit main_window_data(::GdkWindow* window) : rc_(window), texture_(rc_)
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
        rc_.clear(0.0f, 0.0f, 1.0f, 1.0f);

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
    render_context rc_;
    texture texture_;

    main_window_data(const main_window_data&);
    main_window_data& operator=(const main_window_data&);
};


#include <iostream>

void destroy(GtkWidget*, gpointer)
{
    ::gtk_main_quit();
}

void realize(::GtkWidget* widget, ::gpointer user_data)
{
    main_window_data*& pimpl = *static_cast<main_window_data**>(user_data);
    try
    {
        pimpl = new main_window_data(widget->window);
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
