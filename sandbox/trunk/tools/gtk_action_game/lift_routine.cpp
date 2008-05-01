// lift_routine.cpp: the routine for lifts

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "lift_routine.hpp"
#include "collision_utility.hpp"

namespace
{

inline bool on_characters(const game_character& c1, const game_character& c2)
{
    if (c2.slope == slope_type::none)
        return on_rects(c1.bounds(), c2.bounds());
    else
    {
        if (c1.vy != 0.0f)
            return false;

        const rect& r2 = c2.bounds();

        if ((r2.x <= c1.x) && (c1.x < r2.x + r2.lx))
        {
            float dx = c1.x - c2.x;
            float height = c2.slope_height(dx);

            return r2.y + height == c1.y;
        }
        else
            return false;
    }
}

} // namespace

void move_y(game_system* game, game_character* c, float vy)
{
    float old_y = c->y;
    float new_y = old_y + vy;

    rect br = c->bounds();
    rect r = sweep_y(br, vy);

    character_list& ls = game->characters;
    for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
    {
        game_character* c2 = i->get();

        // itself
        if (c2 == c)
            continue;

        if ((c->slope != slope_type::none) && (c2->y > old_y))
        {
            const rect& r2 = c2->bounds();

            if ((c2->vy == 0.0f) && on_characters(*c2, *c))
            {
                float dx = c2->x - c->x;
                float height = c->slope_height(dx);

                c2->y = new_y + height;
            }
            else if ((new_y <= r2.y) && (r2.y < new_y + br.ly) &&
                (r.x <= c2->x) && (c2->x < r.x + r.lx) )
            {
                float dx = c2->x - c->x;
                float height = c->slope_height(dx);
                float y1 = new_y + height;

                if (r2.y < y1)
                {
                    c2->vy = 0.0f;
                    c2->y = y1;
                }
            }
        }
        else
        {
            const rect& r2 = c2->bounds();

            if ((c2->vy == 0.0f) && on_rects(r2, br))
            {
                if (c2->attrs.test(char_attr::block))
                {
                    if (vy < 0.0f)
                        c->y = new_y;
                    move_y(game, c2, new_y + br.ly - r2.y);
                    c->y = old_y;
                }
                else
                    c2->y = new_y + br.ly;
            }
            else if (intersect_rects(r, r2))
            {
                if (vy < 0.0f)
                {
                    if (c2->attrs.test(char_attr::block))
                    {
                        c2->vy = 0.0f;
                        c->y = new_y;
                        move_y(game, c2, new_y - r2.ly - r2.y);
                        c->y = old_y;
                    }
                    else
                    {
                        c2->vy = vy;
                        c2->y = new_y - r2.ly;
                    }
                }
                else
                {
                    c2->vy = 0.0f;
                    if (c2->attrs.test(char_attr::block))
                        move_y(game, c2, new_y + br.ly - r2.y);
                    else
                        c2->y = new_y + br.ly;
                }
            }
        }
    }

    c->y = new_y;
}

bool loop_lift_routine(game_system* game, game_character* c)
{
    const float max_y = static_cast<float>(game->screen_height);
    const float min_y = -64.0f;

    float y = c->y;

    y += c->vy;

    if (y > max_y)
    {
        move_y(game, c, max_y - c->y);
        c->y = min_y;
        move_y(game, c, y - max_y);
    }
    else if (y < min_y)
    {
        move_y(game, c, min_y - c->y);
        c->y = max_y;
        move_y(game, c, y - min_y);
    }
    else
        move_y(game, c, c->vy);

    return true;
}
