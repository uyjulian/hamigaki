// physics_types.hpp: types for physics

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PHYSICS_TYPES_HPP
#define PHYSICS_TYPES_HPP

#include <boost/serialization/nvp.hpp>

template<class T>
struct rectangle
{
    T x;
    T y;
    T lx;
    T ly;

    rectangle() : x(), y(), lx(), ly() {}

    rectangle(T x, T y, T lx, T ly)
        : x(x), y(y), lx(lx), ly(ly)
    {
    }
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

namespace boost { namespace serialization {

template<class Archive, class T>
inline void serialize(
    Archive& ar, rectangle<T>& r, const unsigned int file_version)
{
    ar & make_nvp("x", r.x);
    ar & make_nvp("y", r.y);
    ar & make_nvp("lx", r.lx);
    ar & make_nvp("ly", r.ly);
}

} } // End namespaces serialization, boost.

#endif // PHYSICS_TYPES_HPP
