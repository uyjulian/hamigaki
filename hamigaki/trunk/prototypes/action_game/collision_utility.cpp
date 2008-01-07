// collision_utility.cpp: utilities for collision detection

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "collision_utility.hpp"

slope_type::values
current_slope(const game_character& c, const character_list& ls)
{
    if ((c.width == 0.0f) || (c.height == 0.0f))
        return slope_type::none;

    const rect& r = c.bounds();

    typedef character_list::const_iterator iter_type;
    for (iter_type i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        game_character* c2 = i->get();

        // itself
        if (c2 == &c)
            continue;

        if (!c2->attrs.test(char_attr::block) ||
            (c2->slope == slope_type::none) )
        {
            continue;
        }

        const rect& r2 = c2->bounds();
        if ((r2.x <= c.x) && (c.x <  r2.x + r2.lx) && (r2.y <  c.y))
        {
            if (c.y < r2.y + r2.ly)
                return c2->slope;
            else if (c.y == r2.y + r2.ly)
            {
                if (r.x <= r2.x)
                {
                    if (c2->slope == slope_type::right_down)
                        return c2->slope;
                }
                else if (r2.x + r2.lx <= r.x + r.lx)
                {
                    if (c2->slope == slope_type::left_down)
                        return c2->slope;
                }
            }
        }
    }

    return slope_type::none;
}

bool is_on_floor(const game_character& c, const character_list& ls)
{
    if ((c.vy != 0.0f) || (c.width == 0.0f) || (c.height == 0.0f))
        return false;

    const rect& r = c.bounds();

    typedef character_list::const_iterator iter_type;
    for (iter_type i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        game_character* c2 = i->get();

        // itself
        if (c2 == &c)
            continue;

        if (!c2->attrs.test(char_attr::block))
            continue;

        if (c2->slope != slope_type::none)
        {
            const rect& r2 = c2->bounds();
            if ((r2.x > c.x) || (c.x >= r2.x + r2.lx))
                continue;

            float dx = c.x - c2->x;
            float height = c2->slope_height(dx);
            if (c2->y + height == c.y)
                return true;
        }
        else if (on_rects(r, c2->bounds()))
            return true;
    }

    return false;
}

void process_collisions(game_system& game, game_character& c)
{
    if ((c.width == 0.0f) || (c.height == 0.0f))
        return;

    character_list& ls = game.characters;
    const rect& r = c.bounds();

    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        game_character* c2 = i->get();

        // itself
        if (c2 == &c)
            continue;

        const rect& r2 = c2->bounds();

        if (c2->attrs.test(char_attr::player))
        {
            if (c.attrs.test(char_attr::enemy))
            {
                const rect& ar = c2->attack_rect();
                if (intersect_rects(ar, r))
                {
                    if (c.on_hit)
                    {
                        c.on_hit(&game, &c, c2);
                        continue;
                    }
                }

                rect u = r;
                u.ly *= 0.5f;
                u.y += u.ly;

                float x1 = r2.x;
                float y1 = r2.y;
                float x2 = r2.x + r2.lx;
                float y2 = r2.y + r2.ly;

                if ( (includes_point(u, x1, y1) || includes_point(u, x2, y1)) &&
                    !(includes_point(u, x1, y2) || includes_point(u, x2, y2)) &&
                    ((current_slope(*c2, ls) == slope_type::none) ||
                     !is_on_floor(*c2, ls) ) )
                {
                    if (c.on_stomp)
                        c.on_stomp(&game, &c, c2);
                    continue;
                }
            }

            if (intersect_rects(r, r2))
            {
                if (c.on_collide_player)
                    c.on_collide_player(&game, &c, c2);
            }
            else if (r2.y == r.y + r.ly)
            {
                if (c.on_touch_player)
                    c.on_touch_player(&game, &c, c2);
            }
        }
        else if (c2->attrs.test(char_attr::enemy))
        {
            if (intersect_rects(r, r2))
            {
                if (c.on_collide_enemy)
                    c.on_collide_enemy(&game, &c, c2);
            }
        }
        else if (c2->attrs.test(char_attr::weapon))
        {
            if (c.attrs.test(char_attr::enemy))
            {
                if (intersect_rects(r, r2))
                {
                    if (c.on_hit)
                        c.on_hit(&game, &c, c2);
                }
            }
        }
    }
}
