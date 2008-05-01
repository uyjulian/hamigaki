// velocity_routine.cpp: the velocity-base moving routine

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "velocity_routine.hpp"
#include "collision_utility.hpp"

namespace
{

void vx_routine(game_system* game, game_character* c)
{
    if ((c->width == 0.0f) || (c->height == 0.0f))
    {
        c->x += c->vx;
        return;
    }

    character_list& ls = game->characters;
    bool on_floor = is_on_floor(*c, ls);

    float x = c->x;
    x += c->vx;

    const rect& br = c->bounds();
    rect r = sweep_x(br, c->vx);

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
        game_character* c2 = i->get();

        // itself
        if (c2 == c)
            continue;

        if (!c2->attrs.test(char_attr::block))
            continue;

        const rect& r2 = c2->bounds();

        if ((c2->slope == slope_type::left_down)
            && (c->x < r2.x + r2.lx) )
        {
            ;
        }
        else if ((c2->slope == slope_type::right_down) &&
            (r2.x < c->x) )
        {
            ;
        }
        else if (
            c->attrs.test(char_attr::player) &&
            intersect_rects(br, r2) && !intersect_rects(er, r2) )
        {
            c->x += 2.0f;
            return;
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
            game_character* c2 = i->get();

            // itself
            if (c2 == c)
                continue;

            if (!c2->attrs.test(char_attr::block) ||
                (c2->slope == slope_type::none) )
            {
                continue;
            }

            const rect& r2 = c2->bounds();
            if ((r2.x <= c->x) && (c->x <  r2.x + r2.lx) &&
                (r2.y <  c->y) && (c->y <= r2.y + r2.ly) )
            {
                float dx = x - c2->x;
                float w = c2->width*0.5f;
                if (dx < -w)
                    dx = -w;
                else if (dx > w)
                    dx = w;

                float height = c2->slope_height(dx);
                c->y = c2->y + height;
                break;
            }
        }

        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            game_character* c2 = i->get();

            // itself
            if (c2 == c)
                continue;

            if (!c2->attrs.test(char_attr::block) ||
                (c2->slope == slope_type::none) )
            {
                continue;
            }

            const rect& r2 = c2->bounds();
            if ((r2.x <= x) && (x <  r2.x + r2.lx) &&
                (r2.y <  c->y) && (c->y <= r2.y + r2.ly) )
            {
                float dx = x - c2->x;
                float height = c2->slope_height(dx);
                c->y = c2->y + height;
                break;
            }
        }
    }

    c->x = x;

    if ((ti != ls.end()) && c->on_collide_block_side)
        c->on_collide_block_side(game, c, ti->get());
}

void vy_down_routine(game_system* game, game_character* c)
{
    float y = c->y;
    y += c->vy;

    const rect& br = c->bounds();
    rect r = sweep_y(br, c->vy);

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
        game_character* c2 = i->get();

        // itself
        if (c2 == c)
            continue;

        if (!c2->attrs.test(char_attr::block))
            continue;

        const rect& r2 = c2->bounds();

        if ((c2->slope != slope_type::none) &&
            (r2.y <= c->y) && (r.y < r2.y + r2.ly) )
        {
            if ((r2.x <= c->x) && (c->x < r2.x + r2.lx))
            {
                float dx = c->x - c2->x;
                float height = c2->slope_height(dx);
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
        else if (intersect_rects(br, r2) && !intersect_rects(er, r2))
            ;
        else if (intersect_rects(r, r2) && !intersect_rects(er, r2))
        {
            float dx = c->x - c2->x;
            float height = c2->slope_height(dx);
            if (height == r2.ly)
            {
                c->vy = 0.0f;
                y = r2.y + height;
                r.y = y;
                r.ly = c->height + c->y - y;
            }
        }
    }

    c->y = y;
}

void vy_up_routine(game_system* game, game_character* c)
{
    float y = c->y;
    y += c->vy;

    bool collision = false;

    character_list& ls = game->characters;
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        game_character* c2 = i->get();

        // itself
        if (c2 == c)
            continue;

        if (!c2->attrs.test(char_attr::block))
            continue;

        const rect& r2 = c2->bounds();

        if ((r2.x <= c->x) && (c->x < r2.x + r2.lx) &&
            (c->y + c->height <= c2->y) && (c2->y < y + c->height) )
        {
            collision = true;
            c->vy = -c->vy * 0.5f;
            y = r2.y - c->height;

            if (c->attrs.test(char_attr::player) ||
                c->attrs.test(char_attr::breaker) )
            {
                if (c2->on_hit_from_below)
                    c2->on_hit_from_below(game, c2, c);
            }
        }
    }

    if (collision)
    {
        c->y = y;
        return;
    }

    const rect& br = c->bounds();
    rect r = sweep_y(br, c->vy);

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
        game_character* c2 = i->get();

        // itself
        if (c2 == c)
            continue;

        if (!c2->attrs.test(char_attr::block))
            continue;

        const rect& r2 = c2->bounds();

        if (intersect_rects(br, r2) && !intersect_rects(er, r2))
            ;
        else if (intersect_rects(r, r2) && !intersect_rects(er, r2))
        {
            if (c->x < c2->x)
                c->x = r2.x - c->width*0.5f;
            else
                c->x = r2.x + r2.lx + c->width*0.5f;
        }
    }

    c->y = y;
}

void vy_routine(game_system* game, game_character* c)
{
    if ((c->width == 0.0f) || (c->height == 0.0f))
        c->y += c->vy;
    else if (c->vy <= 0.0f)
        vy_down_routine(game, c);
    else
        vy_up_routine(game, c);
}

} // namespace

bool velocity_routine(game_system* game, game_character* c)
{
    if (!c->speed_routine.empty())
    {
        // Note:
        // This copy guarantees the lifetime until the call is completed.
        speed_routine_type sp = c->speed_routine;
        if (!sp(game, c))
            c->speed_routine.clear();
    }

    bool on_floor = is_on_floor(*c, game->characters);

    vx_routine(game, c);

    if (!on_floor)
    {
        c->vy += game->gravity;
        c->vy = (std::max)(c->vy, game->min_vy);
    }

    vy_routine(game, c);

    return true;
}

bool fly_routine(game_system* game, game_character* c)
{
    if (!c->speed_routine.empty())
    {
        // Note:
        // This copy guarantees the lifetime until the call is completed.
        speed_routine_type sp = c->speed_routine;
        if (!sp(game, c))
            c->speed_routine.clear();
    }

    vx_routine(game, c);
    vy_routine(game, c);

    return true;
}
