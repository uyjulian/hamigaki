// turn_routine.cpp: the routine for going straight with a turn

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "turn_routine.hpp"
#include "collision_utility.hpp"

bool turn_routine(game_system* game, game_character* c)
{
    character_list& ls = game->characters;
    float new_x = c->x + c->vx;

    if (is_on_floor(*c, ls))
    {
        for (character_iterator i = ls.begin(), end = ls.end(); i != end; ++i)
        {
            // itself
            if (&*i == c)
                continue;

            if (!i->attrs.test(char_attr::block))
                continue;

            const rect& r2 = i->bounds();
            if ((r2.x <= new_x) && (new_x < r2.x + r2.lx) &&
                (c->y == r2.y + r2.ly) )
            {
                return true;
            }
        }
    }

    c->vx = -c->vx;
    c->back = !c->back;

    return true;
}
