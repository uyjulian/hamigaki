// draw.hpp: draw functions

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef DRAW_HPP
#define DRAW_HPP

#include "direct3d_device9.hpp"

struct transformed_lit_vertex
{
    float x, y, z, rhw;
    unsigned long color;
};

inline void draw_rectangle(direct3d_device9& device,
    float x, float y, float z,
    float width, float height, unsigned long color)
{
    const transformed_lit_vertex vertices[] =
    {
        { x,       y,        z, 1.0f, color },
        { x+width, y,        z, 1.0f, color },
        { x,       y+height, z, 1.0f, color },
        { x+width, y+height, z, 1.0f, color }
    };

    device.set_vertex_format(D3DFVF_XYZRHW|D3DFVF_DIFFUSE);
    device.clear_texture(0);
    device.draw_primitive(
        D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertices[0]));
}

#endif // DRAW_HPP
