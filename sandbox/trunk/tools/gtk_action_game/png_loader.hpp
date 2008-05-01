// png_loader.hpp: PNG loader

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PNG_LOADER_HPP
#define PNG_LOADER_HPP

#include "direct3d_device9.hpp"
#include <string>

direct3d_texture9
create_png_texture(direct3d_device9& device, const std::string& filename);

#endif // PNG_LOADER_HPP
