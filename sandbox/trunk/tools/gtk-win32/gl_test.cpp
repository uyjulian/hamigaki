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

    void begin_scene()
    {
        assert(::wglGetCurrentDC() == 0);
        assert(::wglGetCurrentContext() == 0);

        ::wglMakeCurrent(dc_.get(), handle_);
    }

    void end_scene()
    {
        assert(::wglGetCurrentDC() == dc_.get());
        assert(::wglGetCurrentContext() == handle_);

        ::wglMakeCurrent(0, 0);
    }

    void swap_buffers()
    {
        assert(::wglGetCurrentDC() == dc_.get());
        assert(::wglGetCurrentContext() == handle_);

        ::SwapBuffers(dc_.get());
    }

    void clear(float r, float g, float b, float a)
    {
        assert(::wglGetCurrentDC() == dc_.get());
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

class scoped_scene
{
public:
    explicit scoped_scene(render_context& rc) : rc_(rc)
    {
        rc_.begin_scene();
    }

    ~scoped_scene()
    {
        rc_.end_scene();
    }

private:
    render_context& rc_;

    scoped_scene(const scoped_scene&);
    scoped_scene& operator=(const scoped_scene&);
};


#include <iostream>

void destroy(GtkWidget*, gpointer)
{
    ::gtk_main_quit();
}

void realize(::GtkWidget* widget, ::gpointer user_data)
{
    render_context*& pimpl = *static_cast<render_context**>(user_data);
    try
    {
        pimpl = new render_context(widget->window);
    }
    catch (...)
    {
        pimpl = 0;
    }
}

void unrealize(::GtkWidget*, ::gpointer user_data)
{
    render_context*& pimpl = *static_cast<render_context**>(user_data);
    delete pimpl;
    pimpl = 0;
}

::gboolean draw(::gpointer user_data)
{
    if (render_context*& pimpl = *static_cast<render_context**>(user_data))
    {
        try
        {
            scoped_scene scene(*pimpl);
            pimpl->clear(0.0f, 0.0f, 1.0f, 1.0f);
            pimpl->swap_buffers();
        }
        catch (...)
        {
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
        ::gtk_init(&argc, &argv);

        ::GtkWidget* window = ::gtk_window_new(GTK_WINDOW_TOPLEVEL);
        if (window == 0)
            throw std::runtime_error("gtk_window_new() failed");

        render_context* pimpl = 0;
        connect_signal(window, "destroy", G_CALLBACK(destroy), &pimpl);
        connect_signal(window, "realize", G_CALLBACK(realize), &pimpl);
        connect_signal(window, "unrealize", G_CALLBACK(unrealize), &pimpl);
        ::g_idle_add(&draw, &pimpl);

        ::gtk_widget_show(window);
        ::gtk_main();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
