// png_loader.cpp: PNG loader

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "png_loader.hpp"
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/gil/image_view_factory.hpp>

namespace gil = boost::gil;

direct3d_texture9
create_png_texture(direct3d_device9& device, const std::string& filename)
{
    const gil::point2<std::ptrdiff_t>& dim
        = boost::gil::png_read_dimensions(filename);

    direct3d_texture9 texture =
        device.create_texture(
            dim.x, dim.y, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED
        );

    direct3d_texture9::scoped_lock locking(texture, 0);

    typedef gil::bgra8_pixel_t pixel_t;
    typedef gil::type_from_x_iterator<pixel_t*>::view_t view_t;

    view_t out = gil::interleaved_view(
        dim.x, dim.y,
        static_cast<pixel_t*>(locking.address()), locking.pitch()
    );

    gil::png_read_and_convert_view(filename, out);

    return texture;
}
