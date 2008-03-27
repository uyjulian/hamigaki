// keyboard.hpp: the keyboard class

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_KEYBOARD_HPP
#define HAMIGAKI_KEYBOARD_HPP

#include <boost/config.hpp>

#include <bitset>
#include <stdexcept>
#include <gtk/gtk.h>

namespace hamigaki
{

namespace detail
{

class scoped_connect_signal
{
public:
	scoped_connect_signal(gpointer obj, gulong id)
		: obj_(obj), id_(id)
	{
		if (id_ == 0)
			throw std::runtime_error("cannot connect signal");
	}

	~scoped_connect_signal()
	{
		::g_signal_handler_disconnect(obj_, id_);
	}

private:
	gpointer obj_;
	gulong id_;

	scoped_connect_signal(const scoped_connect_signal&);
	scoped_connect_signal& operator=(const scoped_connect_signal&);
};

} // namespace detail

class keyboard
{
public:
	explicit keyboard(GtkWidget* widget)
		: widget_(widget)
		, key_press_(G_OBJECT(widget),
			::g_signal_connect(
				G_OBJECT(widget), "key-press-event",
				G_CALLBACK(&keyboard::key_press_callback), this
			)
		)
		, key_release_(G_OBJECT(widget),
			::g_signal_connect(
				G_OBJECT(widget), "key-release-event",
				G_CALLBACK(&keyboard::key_release_callback), this
			)
		)
	{
	}

	bool pushed(int vkey) const
	{
#if defined(BOOST_WINDOWS)
		return table_.test(static_cast<std::size_t>(vkey));
#else
		Display* dpy = GDK_SCREEN_XDISPLAY(::gtk_window_get_screen(widget_);
		int key = ::XKeysymToKeycode(dpy, vkey);
		return table_.test(static_cast<std::size_t>(key));
#endif
	}

private:
	GtkWidget* widget_;
	std::bitset<256> table_;
	detail::scoped_connect_signal key_press_;
	detail::scoped_connect_signal key_release_;

	void press_key(int key)
	{
		table_.set(static_cast<std::size_t>(key));
	}

	void release_key(int key)
	{
		table_.reset(static_cast<std::size_t>(key));
	}

	static gboolean key_press_callback(
		GtkWidget* widget, GdkEventKey* event, keyboard* this_ptr)
	{
		this_ptr->press_key(event->hardware_keycode);
	    return TRUE;
	}

	static gboolean key_release_callback(
		GtkWidget* widget, GdkEventKey* event, keyboard* this_ptr)
	{
		this_ptr->release_key(event->hardware_keycode);
	    return TRUE;
	}

	keyboard(const keyboard&);
	keyboard& operator=(const keyboard&);
};

} // namespace hamigaki

#endif // HAMIGAKI_KEYBOARD_HPP
