// render_context.hpp: the OpenGL rendering context class

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_RENDER_CONTEXT_HPP
#define HAMIGAKI_RENDER_CONTEXT_HPP

#include <boost/shared_ptr.hpp>
#include <gtk/gtk.h>

namespace hamigaki
{

class render_context
{
public:
	explicit render_context(GtkWidget* widget);
	~render_context();
	void select();
	void swap_buffers();

private:
	class impl;
	boost::shared_ptr<impl> pimpl_;
};

} // namespace hamigaki

#endif // HAMIGAKI_RENDER_CONTEXT_HPP
