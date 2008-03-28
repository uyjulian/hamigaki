// render_context_test.cpp: the test program for the class render_context

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <boost/config.hpp>

#include "render_context.hpp"
#include <iostream>
#include <stdexcept>

#if defined(BOOST_WINDOWS)
	#include <windows.h>
#endif
#include <GL/gl.h>

class main_window_data
{
public:
	explicit main_window_data(GtkWindow* window) : rc_(window)
	{
	}

	void render()
	{
		rc_.select();

		::glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
		::glClear(GL_COLOR_BUFFER_BIT);

		rc_.swap_buffers();
	}

private:
	hamigaki::render_context rc_;

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
		pimpl = new main_window_data(GTK_WINDOW(widget));
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
