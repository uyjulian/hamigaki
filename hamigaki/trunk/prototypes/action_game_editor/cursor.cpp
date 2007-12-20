// cursor.cpp: draw a box-cursor to the texture

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "cursor.hpp"
#include <boost/cstdint.hpp>
#include <boost/scoped_array.hpp>
#include <stdexcept>

direct3d_texture9 create_cursor_texture(
    direct3d_device9& device, std::size_t width, std::size_t height)
{
    if ((width < 3) || (height < 3))
        throw std::invalid_argument("the size of the cursor is too small");

    const std::size_t pitch = 4*width;

    boost::scoped_array<boost::uint32_t> buffer0(new boost::uint32_t[width]);
    std::fill_n(buffer0.get(), width, 0xFF000000ul);

    boost::scoped_array<boost::uint32_t> buffer1(new boost::uint32_t[width]);
    std::fill_n(buffer1.get(), width, 0xFFFFFFFFul);
    buffer1[0] = 0xFF000000ul;
    buffer1[width-1] = 0xFF000000ul;

    boost::scoped_array<boost::uint32_t> buffer2(new boost::uint32_t[width]);
    std::fill_n(buffer2.get(), width, 0xFF000000ul);
    buffer2[1] = 0xFFFFFFFFul;
    buffer2[width-2] = 0xFFFFFFFFul;

    boost::scoped_array<boost::uint32_t> buffer3(new boost::uint32_t[width]);
    std::memset(buffer3.get(), 0, pitch);
    buffer3[0] = 0xFF000000ul;
    buffer3[1] = 0xFFFFFFFFul;
    buffer3[2] = 0xFF000000ul;
    buffer3[width-3] = 0xFF000000ul;
    buffer3[width-2] = 0xFFFFFFFFul;
    buffer3[width-1] = 0xFF000000ul;

    direct3d_texture9 texture =
        device.create_texture(
            width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED
        );

    direct3d_texture9::scoped_lock locking(texture, 0);

    unsigned char* dst = static_cast<unsigned char*>(locking.address());

    std::memcpy(dst, buffer0.get(), pitch);
    dst += locking.pitch();

    std::memcpy(dst, buffer1.get(), pitch);
    dst += locking.pitch();

    std::memcpy(dst, buffer2.get(), pitch);
    dst += locking.pitch();

    for (std::size_t i = 3; i < height-3; ++i)
    {
        std::memcpy(dst, buffer3.get(), pitch);
        dst += locking.pitch();
    }

    std::memcpy(dst, buffer2.get(), pitch);
    dst += locking.pitch();

    std::memcpy(dst, buffer1.get(), pitch);
    dst += locking.pitch();

    std::memcpy(dst, buffer0.get(), pitch);

    return texture;
}
