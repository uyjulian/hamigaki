// collision_utility.hpp: utilities for collision detection

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef COLLISION_UTILITY_HPP
#define COLLISION_UTILITY_HPP

#include "game_character.hpp"
#include "game_system.hpp"

inline bool on_rects(const rect& r1, const rect& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y == r2.y + r2.ly) ;
}

inline rect sweep_x(const rect& r, float dx)
{
    if (r.lx == 0.0f)
        return r;
    else if (dx < 0.0f)
        return rect(r.x+dx, r.y, r.lx-dx, r.ly);
    else
        return rect(r.x, r.y, r.lx+dx, r.ly);
}

inline rect sweep_y(const rect& r, float dy)
{
    if (r.ly == 0.0f)
        return r;
    else if (dy < 0.0f)
        return rect(r.x, r.y+dy, r.lx, r.ly-dy);
    else
        return rect(r.x, r.y, r.lx, r.ly+dy);
}

slope_type::values
current_slope(const game_character& c, const character_list& ls);

bool is_on_floor(const game_character& c, const character_list& ls);

#endif // COLLISION_UTILITY_HPP
