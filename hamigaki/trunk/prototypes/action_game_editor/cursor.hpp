// cursor.hpp: draw a box-cursor to the texture

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef CURSOR_HPP
#define CURSOR_HPP

#include "direct3d_device9.hpp"

direct3d_texture9 create_cursor_texture(
    direct3d_device9& device, std::size_t width, std::size_t height);

#endif // CURSOR_HPP
