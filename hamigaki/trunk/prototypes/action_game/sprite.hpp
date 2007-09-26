// sprite.hpp: sprite functions

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SPRITE_HPP
#define SPRITE_HPP

#include "direct3d_device9.hpp"
#include "sprite_form.hpp"

struct transformed_vertex
{
    float x, y, z, rhw;
    unsigned long color;
    float tu, vu;
};

inline void draw_sprite(direct3d_device9& device,
    float x, float y, float z, direct3d_texture9& texture,
    unsigned long color = 0xFFFFFFFFul)
{
    const ::D3DSURFACE_DESC& desc = texture.description(0);
    float width = static_cast<float>(desc.Width);
    float height = static_cast<float>(desc.Height);;

    x -= 0.5f;
    y -= 0.5f;

    const transformed_vertex vertices[] =
    {
        { x,       y,        z, 1.0f, color, 0.0f, 0.0f },
        { x+width, y,        z, 1.0f, color, 1.0f, 0.0f },
        { x,       y+height, z, 1.0f, color, 0.0f, 1.0f },
        { x+width, y+height, z, 1.0f, color, 1.0f, 1.0f }
    };

    device.set_vertex_format(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
    device.set_texture(texture, 0);
    device.draw_primitive(
        D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
}

inline void draw_sprite(direct3d_device9& device,
    float x, float y, float z,
    direct3d_texture9& texture, int tx, int ty, int tw, int th,
    boost::uint32_t options, unsigned long color = 0xFFFFFFFFul)
{
    const ::D3DSURFACE_DESC& desc = texture.description(0);
    float tu1 = static_cast<float>(tx) / static_cast<float>(desc.Width);
    float tu2 = static_cast<float>(tx+tw) / static_cast<float>(desc.Width);
    float tv1 = static_cast<float>(ty) / static_cast<float>(desc.Height);
    float tv2 = static_cast<float>(ty+th) / static_cast<float>(desc.Height);

    if ((options & sprite_options::back) != 0)
        std::swap(tu1, tu2);
    if ((options & sprite_options::upside_down) != 0)
        std::swap(tv1, tv2);

    x -= 0.5f;
    y -= 0.5f;

    const transformed_vertex vertices[] =
    {
        { x,    y,    z, 1.0f, color, tu1, tv1 },
        { x+tw, y,    z, 1.0f, color, tu2, tv1 },
        { x,    y+th, z, 1.0f, color, tu1, tv2 },
        { x+tw, y+th, z, 1.0f, color, tu2, tv2 }
    };

    device.set_vertex_format(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1);
    device.set_texture(texture, 0);
    device.draw_primitive(
        D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
}

#endif // SPRITE_HPP
