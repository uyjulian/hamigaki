// sprite.hpp: sprite functions

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "direct3d_device9.hpp"

struct transformed_vertex
{
    float x, y, z, rhw;
    float tu, vu;
};

inline void draw_sprite(direct3d_device9& device,
    float x, float y, float z, direct3d_texture9& texture)
{
    const ::D3DSURFACE_DESC& desc = texture.description(0);
    float width = static_cast<float>(desc.Width);
    float height = static_cast<float>(desc.Height);;

    const transformed_vertex vertices[] =
    {
        { x,       y,        z, 1.0f, 0.0f, 0.0f },
        { x+width, y,        z, 1.0f, 1.0f, 0.0f },
        { x,       y+height, z, 1.0f, 0.0f, 1.0f },
        { x+width, y+height, z, 1.0f, 1.0f, 1.0f }
    };

    device.set_vertex_format(D3DFVF_XYZRHW|D3DFVF_TEX1);
    device.set_texture(texture, 0);
    device.draw_primitive(
        D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
}

#endif // SPRITE_HPP
