// velocity_routine.cpp: the velocity-base moving routine

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "velocity_routine.hpp"
#include "game_character.hpp"
#include "game_system.hpp"

namespace
{

inline bool on_rects(const rect& r1, const rect& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y == r2.y + r2.ly) ;
}

inline rect sweep_x(const rect& r, float dx)
{
    if (dx < 0.0f)
        return rect(r.x+dx, r.y, r.lx-dx, r.ly);
    else
        return rect(r.x, r.y, r.lx+dx, r.ly);
}

inline rect sweep_y(const rect& r, float dy)
{
    if (dy < 0.0f)
        return rect(r.x, r.y+dy, r.lx, r.ly-dy);
    else
        return rect(r.x, r.y, r.lx, r.ly+dy);
}

slope_type::values
current_slope(const game_character& c, const character_list& ls)
{
    typedef character_list::const_iterator iter_type;
    for (iter_type i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == &c)
            continue;

        if (!i->attrs.test(char_attr::block) || (i->slope == slope_type::none))
            continue;

        const rect& r2 = i->bounds();
        if ((r2.x <= c.x) && (c.x <  r2.x + r2.lx) &&
            (r2.y <  c.y) && (c.y <= r2.y + r2.ly) )
        {
            return i->slope;
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

} // namespace

void vx_routine(game_system* game, game_character* c)
{
    character_list& ls = game->characters;
    bool on_floor = is_on_floor(*c, ls);

    float x = c->x;
    x += c->vx;

    rect r = sweep_x(c->bounds(), c->vx);

    slope_type::values slope = current_slope(*c, ls);

    rect er;
    if (slope == slope_type::left_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x;
        er.y = c->y;
        er = sweep_x(er, c->vx);
    }
    else if (slope == slope_type::right_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x - er.lx;
        er.y = c->y;
        er = sweep_x(er, c->vx);
    }

    character_iterator ti = ls.end();
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        const rect& r2 = i->bounds();

        if ((i->slope == slope_type::left_down)
            && (c->x < r2.x + r2.lx) )
        {
            ;
        }
        else if ((i->slope == slope_type::right_down) &&
            (r2.x < c->x) )
        {
            ;
        }
        else if (intersect_rects(r, r2) && !intersect_rects(er, r2))
        {
            ti = i;
            if (c->vx < 0.0f)
            {
                r.x = r2.x + r2.lx;
                x = r.x + c->width * 0.5f;
                r.lx = c->width + c->x - x;
            }
            else
            {
                x = r2.x - c->width * 0.5f;
                r.lx = c->width + x - c->x;
            }
        }
    }

    if (on_floor)
    {
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            if (!i->attrs.test(char_attr::block) ||
                (i->slope == slope_type::none) )
                continue;

            const rect& r2 = i->bounds();
            if ((r2.x <= c->x) && (c->x <  r2.x + r2.lx) &&
                (r2.y <  c->y) && (c->y <= r2.y + r2.ly) )
            {
                float dx = x - i->x;
                float w = i->width*0.5f;
                if (dx < -w)
                    dx = -w;
                else if (dx > w)
                    dx = w;

                float height = i->slope_height(dx);
                c->y = i->y + height;
                break;
            }
        }

        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            if (!i->attrs.test(char_attr::block) ||
                (i->slope == slope_type::none) )
                continue;

            const rect& r2 = i->bounds();
            if ((r2.x <= x) && (x <  r2.x + r2.lx) &&
                (r2.y <  c->y) && (c->y <= r2.y + r2.ly) )
            {
                float dx = x - i->x;
                float height = i->slope_height(dx);
                c->y = i->y + height;
                break;
            }
        }
    }

    c->x = x;

    if ((ti != ls.end()) && c->on_collide_block_side)
        c->on_collide_block_side(game, c, &*ti);
}

void vy_routine(game_system* game, game_character* c)
{
    float y = c->y;
    y += c->vy;

    rect r = sweep_y(c->bounds(), c->vy);

    character_list& ls = game->characters;
    slope_type::values slope = current_slope(*c, ls);

    rect er;
    if (slope == slope_type::left_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x;
        er.y = c->y;
        er = sweep_y(er, c->vy);
    }
    else if (slope == slope_type::right_down)
    {
        er.lx = c->width * 0.5f;
        er.ly = er.lx;
        er.x = c->x - er.lx;
        er.y = c->y;
        er = sweep_y(er, c->vy);
    }

    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        // itself
        if (&*i == c)
            continue;

        if (!i->attrs.test(char_attr::block))
            continue;

        const rect& r2 = i->bounds();

        if ((i->slope != slope_type::none) &&
            (r2.y <= c->y) && (r.y < r2.y + r2.ly) )
        {
            if ((r2.x <= c->x) && (c->x < r2.x + r2.lx))
            {
                float dx = c->x - i->x;
                float height = i->slope_height(dx);
                float y2 = r2.y + height;

                if (y < y2)
                {
                    c->vy = 0.0f;
                    y = y2;
                    r.y = y;
                    r.ly = c->height + c->y - r.y;
                }
            }
        }
        else if (intersect_rects(r, r2) && !intersect_rects(er, r2))
        {
            if (c->vy < 0.0f)
            {
                c->vy = 0.0f;
                y = r2.y + r2.ly;
                r.y = y;
                r.ly = c->height + c->y - y;
            }
            else
            {
                c->vy = -c->vy * 0.5f;
                y = r2.y - c->height;
                r.ly = c->height + y - c->y;
            }
        }
    }

    c->y = y;
}

void velocity_routine(game_system* game, game_character* c)
{
    vx_routine(game, c);

    c->vy += game->gravity;
    c->vy = (std::max)(c->vy, game->min_vy);

    vy_routine(game, c);
}
