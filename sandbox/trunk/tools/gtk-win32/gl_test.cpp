// gl_test.cpp: OpenGL test program for GTK+2/Win32

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <gdk/gdkwin32.h>
#include <gtk/gtk.h>
#include <GL/gl.h>

class scoped_dc
{
public:
    explicit scoped_dc(::GtkWidget* widget)
        : window_(widget->window), gc_(0), hdc_(0)
    {
        gc_ = ::gdk_gc_new(window_);
        hdc_ = ::gdk_win32_hdc_get(window_, gc_, ::GdkGCValuesMask());
    }

    ~scoped_dc()
    {
        ::gdk_win32_hdc_release(window_, gc_, ::GdkGCValuesMask());
        ::gdk_gc_unref(gc_);
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
        : widget_(widget), hrc_(0)
    {
    }

    void create_gl_context()
    {
        scoped_dc dc(widget_);

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
        hrc_ = 0;
    }

    void swap_buffers()
    {
        if (!hrc_)
            return;

        scoped_dc dc(widget_);
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
    ::HGLRC hrc_;

    gl_window_data(const gl_window_data&);
    gl_window_data& operator=(const gl_window_data&);
};

void destroy(GtkWidget* widget, gpointer user_data)
{
    ::gtk_main_quit();
}

::gboolean
delete_event(::GtkWidget* widget, ::GdkEvent* event, ::gpointer user_data)
{
    gl_window_data& data = *static_cast<gl_window_data*>(user_data);
    data.destroy_gl_context();

    return FALSE;
}

::gboolean draw(::gpointer user_data)
{
    gl_window_data& data = *static_cast<gl_window_data*>(user_data);
    data.clear();
    data.swap_buffers();

    return TRUE;
}

int main(int argc, char* argv[])
{
    ::gtk_init(&argc, &argv);

    ::GtkWidget* window = ::gtk_window_new(GTK_WINDOW_TOPLEVEL);

    ::gtk_widget_show(window);

    gl_window_data data(window);
    data.create_gl_context();

    ::g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);

    ::g_signal_connect(
        G_OBJECT(window), "delete-event", G_CALLBACK(delete_event), &data);

    ::g_idle_add(&draw, &data);

    ::gtk_main();

    return 0;
}
