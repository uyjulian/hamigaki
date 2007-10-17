// physics_types.hpp: types for physics

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PHYSICS_TYPES_HPP
#define PHYSICS_TYPES_HPP

template<class T>
struct rectangle
{
    T x;
    T y;
    T lx;
    T ly;
};
typedef rectangle<float> rect;

struct acceleration
{
    float ax;
    float ay;

    acceleration() : ax(0.0f), ay(0.0f) {}
};

struct velocity
{
    float vx;
    float vy;

    velocity() : vx(0.0f), vy(0.0f) {}
};

#endif // PHYSICS_TYPES_HPP
