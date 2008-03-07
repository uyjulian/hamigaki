// gl_test.cpp: OpenGL test program for GTK+2/Win32

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <gdk/gdkwin32.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
                        #include <iostream>

class scoped_dc
{
public:
    scoped_dc(::GtkWidget* widget, ::GdkGC* gc)
        : window_(widget->window), gc_(gc), hdc_(0)
    {
        hdc_ = ::gdk_win32_hdc_get(window_, gc_, ::GdkGCValuesMask());
    }

    ~scoped_dc()
    {
        ::gdk_win32_hdc_release(window_, gc_, ::GdkGCValuesMask());
    }

    ::HDC get() const
    {
        return hdc_;
    }

private:
    ::GdkWindow* window_;
    ::GdkGC* gc_;
    ::HDC hdc_;

    scoped_dc(const scoped_dc&);
    scoped_dc& operator=(const scoped_dc&);
};

class gl_window_data
{
public:
    explicit gl_window_data(::GtkWidget* widget)
        : widget_(widget), gc_(0), hrc_(0)
    {
    }

    void create_gl_context()
    {
        gc_ = ::gdk_gc_new(widget_->window);
        scoped_dc dc(widget_, gc_);

        ::PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 16;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int fmt = ::ChoosePixelFormat(dc.get(), &pfd);
        ::SetPixelFormat(dc.get(), fmt, &pfd);

        hrc_ = ::wglCreateContext(dc.get());
        ::wglMakeCurrent(dc.get(), hrc_);
    }

    void destroy_gl_context()
    {
        ::wglMakeCurrent(0, 0);
        ::wglDeleteContext(hrc_);
        ::gdk_gc_unref(gc_);
        hrc_ = 0;
    }

    void swap_buffers()
    {
        if (!hrc_)
            return;

        scoped_dc dc(widget_, gc_);
        ::wglMakeCurrent(dc.get(), hrc_);
        ::SwapBuffers(dc.get());
    }

    void clear()
    {
        if (!hrc_)
            return;

        ::glClearColor(0.0, 0.0, 1.0, 0.0);
        ::glClear(GL_COLOR_BUFFER_BIT);
    }

private:
    ::GtkWidget* widget_;
    ::GdkGC* gc_;
    ::HGLRC hrc_;

    gl_window_data(const gl_window_data&);
    gl_window_data& operator=(const gl_window_data&);
};

void destroy(GtkWidget* widget, gpointer user_data)
{
    ::gtk_main_quit();
}

void realize(::GtkWidget* widget, ::gpointer user_data)
{
    gl_window_data& data = *static_cast<gl_window_data*>(user_data);
    data.create_gl_context();
}

void unrealize(::GtkWidget* widget, ::gpointer user_data)
{
    gl_window_data& data = *static_cast<gl_window_data*>(user_data);
    data.destroy_gl_context();
}

::gboolean draw(::gpointer user_data)
{
    gl_window_data& data = *static_cast<gl_window_data*>(user_data);
    data.clear();
    data.swap_buffers();

    return TRUE;
}

template<class Widget, class Arg>
inline ::gulong connect_signal(
    Widget* w, const char* sig, ::GCallback func, Arg* arg)
{
    return ::g_signal_connect(G_OBJECT(w), sig, func, arg);
}

int main(int argc, char* argv[])
{
    ::gtk_init(&argc, &argv);

    ::GtkWidget* window = ::gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gl_window_data data(window);
    connect_signal(window, "destroy", G_CALLBACK(destroy), &data);
    connect_signal(window, "realize", G_CALLBACK(realize), &data);
    connect_signal(window, "unrealize", G_CALLBACK(unrealize), &data);
    ::g_idle_add(&draw, &data);

    ::gtk_widget_show(window);
    ::gtk_main();

    return 0;
}
