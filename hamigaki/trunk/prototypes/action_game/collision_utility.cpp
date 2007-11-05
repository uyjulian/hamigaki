// collision_utility.cpp: utilities for collision detection

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "collision_utility.hpp"

slope_type::values
current_slope(const game_character& c, const character_list& ls)
{
    const rect& r = c.bounds();

    typedef character_list::const_iterator iter_type;
    for (iter_type i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == &c)
            continue;

        if (!i->attrs.test(char_attr::block) || (i->slope == slope_type::none))
            continue;

        const rect& r2 = i->bounds();
        if ((r2.x <= c.x) && (c.x <  r2.x + r2.lx) && (r2.y <  c.y))
        {
            if (c.y < r2.y + r2.ly)
                return i->slope;
            else if (c.y == r2.y + r2.ly)
            {
                if (r.x <= r2.x)
                {
                    if (i->slope == slope_type::right_down)
                        return i->slope;
                }
                else if (r2.x + r2.lx <= r.x + r.lx)
                {
                    if (i->slope == slope_type::left_down)
                        return i->slope;
                }
            }
        }
    }

    return slope_type::none;
}

bool is_on_floor(const game_character& c, const character_list& ls)
{
    if (c.vy != 0.0f)
        return false;

    const rect& r = c.bounds();

    typedef character_list::const_iterator iter_type;
    for (iter_type i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == &c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        if (i->slope != slope_type::none)
        {
            const rect& r2 = i->bounds();
            if ((r2.x > c.x) || (c.x >= r2.x + r2.lx))
                continue;

            float dx = c.x - i->x;
            float height = i->slope_height(dx);
            if (i->y + height == c.y)
                return true;
        }
        else if (on_rects(r, i->bounds()))
            return true;
    }

    return false;
}
