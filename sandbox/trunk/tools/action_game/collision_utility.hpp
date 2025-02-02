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

inline bool includes_point(const rect& r, float x, float y)
{
    return (r.x <= x) && (r.x+r.lx > x) && (r.y <= y) && (r.y+r.ly > y);
}

inline bool intersect_rects(const rect& r1, const rect& r2)
{
    if ((r1.lx == 0.0f) || (r1.ly == 0.0f) ||
        (r2.lx == 0.0f) || (r2.ly == 0.0f) )
    {
        return false;
    }

    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y < r2.y + r2.ly) && (r2.y < r1.y + r1.ly) ;
}

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

void process_collisions(game_system& game, game_character& c);

#endif // COLLISION_UTILITY_HPP
