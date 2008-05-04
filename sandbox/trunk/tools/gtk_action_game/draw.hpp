// draw.hpp: draw functions

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef DRAW_HPP
#define DRAW_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
#endif

#include <GL/gl.h>

inline void draw_rectangle(
    float x, float y, float z,
    float width, float height, unsigned long color)
{
    ::glColor4ub(
        static_cast<unsigned char>((color >> 16) & 0xFF),
        static_cast<unsigned char>((color >>  8) & 0xFF),
        static_cast<unsigned char>((color      ) & 0xFF),
        static_cast<unsigned char>((color >> 24) & 0xFF)
    );

    ::glBegin(GL_TRIANGLE_STRIP);
    ::glVertex3f(x,       y,        z);
    ::glVertex3f(x+width, y,        z);
    ::glVertex3f(x,       y+height, z);
    ::glVertex3f(x+width, y+height, z);
    ::glEnd();
}

#endif // DRAW_HPP
