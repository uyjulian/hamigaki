// gl_test.cpp: OpenGL test program for GTK+2/X11

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glx.h>

class gl_window_data
{
public:
    explicit gl_window_data(::GtkWidget* widget)
        : widget_(widget), gc_(0), ctx_(0)
    {
    }

    void create_gl_context()
    {
        gc_ = ::gdk_gc_new(widget_->window);
        ::Display* dpy = ::gdk_x11_gc_get_xdisplay(gc_);

        ::Window window = GDK_WINDOW_XID(widget_->window);

        ::GdkScreen* scr = ::gdk_drawable_get_screen(widget_->window);
        int scr_num = ::gdk_screen_get_number(scr);

        int attrs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, 0 };
        ::XVisualInfo* vi = ::glXChooseVisual(dpy, scr_num, attrs);

        ctx_ = ::glXCreateContext(dpy, vi, 0, True);
        ::glXMakeCurrent(dpy, window, ctx_);

        ::XFree(vi);
    }

    void destroy_gl_context()
    {
        ::Display* dpy = ::gdk_x11_gc_get_xdisplay(gc_);
        ::glXMakeCurrent(dpy, 0, 0);
        ::glXDestroyContext(dpy, ctx_);
        ::gdk_gc_unref(gc_);
    }

    void swap_buffers()
    {
        ::Display* dpy = ::gdk_x11_gc_get_xdisplay(gc_);
        ::Window window = GDK_WINDOW_XID(widget_->window);
        ::glXSwapBuffers(dpy, window);
    }

    void clear()
    {
        ::glClearColor(0.0, 0.0, 1.0, 0.0);
        ::glClear(GL_COLOR_BUFFER_BIT);
    }

private:
    ::GtkWidget* widget_;
    ::GdkGC* gc_;
    ::GLXContext ctx_;

    gl_window_data(const gl_window_data&);
    gl_window_data& operator=(const gl_window_data&);
};

void destroy(::GtkWidget* widget, ::gpointer user_data)
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

    ::g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), 0);

    ::g_signal_connect(
        G_OBJECT(window), "delete-event", G_CALLBACK(delete_event), &data);

    ::g_idle_add(&draw, &data);

    ::gtk_main();

    return 0;
}
