// physics_types.hpp: types for physics

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PHYSICS_TYPES_HPP
#define PHYSICS_TYPES_HPP

struct rect
{
    float x;
    float y;
    float lx;
    float ly;
};

struct acceleration
{
    float ax;
    float ay;
};

struct velocity
{
    float vx;
    float vy;
};

#endif // PHYSICS_TYPES_HPP
