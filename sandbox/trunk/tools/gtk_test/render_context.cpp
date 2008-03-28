// render_context.cpp: the OpenGL rendering context class

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <boost/config.hpp>

#include "render_context.hpp"
#include <cassert>
#include <stdexcept>

#if defined(BOOST_WINDOWS)
	#include <gdk/gdkwin32.h>
	#include <GL/gl.h>
#else
	#include <gdk/gdkx.h>
	#include <GL/gl.h>
	#include <GL/glx.h>
#endif

namespace hamigaki
{

#if defined(BOOST_WINDOWS)
namespace
{

class device_context
{
public:
	explicit device_context(GdkWindow* window)
		: hwnd_(reinterpret_cast<HWND>(GDK_WINDOW_HWND(window)))
		, handle_(::GetDC(hwnd_))
	{
		if (handle_ == 0)
			throw std::runtime_error("GetDC() failed");
	}

	~device_context()
	{
		::ReleaseDC(hwnd_, handle_);
	}

	HDC get() const
	{
		return handle_;
	}

private:
	HWND hwnd_;
	HDC handle_;

	device_context(const device_context&);
	device_context& operator=(const device_context&);
};

} // namespace

class render_context::impl
{
public:
	explicit impl(GtkWindow* window)
		: dc_(GTK_WIDGET(window)->window), handle_(0)
	{
		PIXELFORMATDESCRIPTOR pfd = {};
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

	~impl()
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

private:
	device_context dc_;
	HGLRC handle_;

	impl(const impl&);
	impl& operator=(const impl&);
};
#else // !defined(BOOST_WINDOWS)
namespace
{

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

} // namespace

class render_context::impl
{
public:
	explicit impl(GtkWindow* window) : window_(window), handle_(0)
	{
		Window win = window_id();

		GdkScreen* scr = ::gtk_window_get_screen(window_);
		Display* dpy = GDK_SCREEN_XDISPLAY(scr);
		int scr_num = ::gdk_screen_get_number(scr);

		int attrs[] = { GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, 0 };
		scoped_x_ptr<XVisualInfo> vi(::glXChooseVisual(dpy, scr_num, attrs));
		if (vi.get() == 0)
			throw std::runtime_error("glXChooseVisual() failed");

		handle_ = ::glXCreateContext(dpy, vi.get(), 0, True);
		if (handle_ == 0)
			throw std::runtime_error("glXCreateContext() failed");
		::glXMakeCurrent(dpy, win, handle_);
	}

	~impl()
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
	GtkWindow* window_;
	GLXContext handle_;

	Window window_id() const
	{
		return GDK_WINDOW_XID(GTK_WIDGET(window_)->window);
	}

	Display* display() const
	{
		return GDK_SCREEN_XDISPLAY(::gtk_window_get_screen(window_));
	}

	impl(const impl&);
	impl& operator=(const impl&);
};
#endif // !defined(BOOST_WINDOWS)

render_context::render_context(GtkWindow* window) : pimpl_(new impl(window))
{
}

render_context::~render_context()
{
}

void render_context::select()
{
	pimpl_->select();
}

void render_context::swap_buffers()
{
	pimpl_->swap_buffers();
}

} // namespace hamigaki
