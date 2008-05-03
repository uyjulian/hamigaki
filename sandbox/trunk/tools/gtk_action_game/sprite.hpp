// sprite.hpp: sprite functions

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SPRITE_HPP
#define SPRITE_HPP

#include <boost/config.hpp>
#include "sprite_form.hpp"
#include "texture.hpp"

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#endif

#include <GL/gl.h>

inline void draw_sprite(
    float x, float y, float z, hamigaki::texture& texture,
    unsigned long color = 0xFFFFFFFFul)
{
    float width = static_cast<float>(texture.width());
    float height = static_cast<float>(texture.height());;

    x -= 0.5f;
    y -= 0.5f;

    ::glColor4b(
        static_cast<unsigned char>((color >> 16) & 0xFF),
        static_cast<unsigned char>((color >>  8) & 0xFF),
        static_cast<unsigned char>((color      ) & 0xFF),
        static_cast<unsigned char>((color >> 24) & 0xFF)
    );

    texture.bind();
    ::glBegin(GL_TRIANGLE_STRIP);
    ::glTexCoord2f(0.0f, 0.0f);
    ::glVertex3f(x,       y,        z);
    ::glTexCoord2f(1.0f, 0.0f);
    ::glVertex3f(x+width, y,        z);
    ::glTexCoord2f(1.0f, 1.0f);
    ::glVertex3f(x,       y+height, z);
    ::glTexCoord2f(0.0f, 1.0f);
    ::glVertex3f(x+width, y+height, z);
    ::glEnd();
}

inline void draw_sprite(
    float x, float y, float z,
    hamigaki::texture& texture, int tx, int ty, int tw, int th,
    boost::uint32_t options, unsigned long color = 0xFFFFFFFFul)
{
    float width = static_cast<float>(texture.width());
    float height = static_cast<float>(texture.height());;

    float tu1 = static_cast<float>(tx) / static_cast<float>(width);
    float tu2 = static_cast<float>(tx+tw) / static_cast<float>(width);
    float tv1 = static_cast<float>(ty) / static_cast<float>(height);
    float tv2 = static_cast<float>(ty+th) / static_cast<float>(height);

    if ((options & sprite_options::back) != 0)
        std::swap(tu1, tu2);
    if ((options & sprite_options::upside_down) != 0)
        std::swap(tv1, tv2);

    x -= 0.5f;
    y -= 0.5f;

    ::glColor4b(
        static_cast<unsigned char>((color >> 16) & 0xFF),
        static_cast<unsigned char>((color >>  8) & 0xFF),
        static_cast<unsigned char>((color      ) & 0xFF),
        static_cast<unsigned char>((color >> 24) & 0xFF)
    );

    texture.bind();
    ::glBegin(GL_TRIANGLE_STRIP);
    ::glTexCoord2f(tu1, tv1);
    ::glVertex3f(x,    y,    z);
    ::glTexCoord2f(tu2, tv1);
    ::glVertex3f(x+tw, y,    z);
    ::glTexCoord2f(tu1, tv2);
    ::glVertex3f(x,    y+th, z);
    ::glTexCoord2f(tu2, tv2);
    ::glVertex3f(x+tw, y+th, z);
    ::glEnd();
}

#endif // SPRITE_HPP
