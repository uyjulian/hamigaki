// side_scrolling_routine.cpp: the routine for side-scrolling camera

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "side_scrolling_routine.hpp"

side_scrolling_routine::side_scrolling_routine(const character_ptr& player)
    : player_(player)
{
}

bool
side_scrolling_routine::operator()(game_system* game, game_character* c) const
{
    if (character_ptr ptr = player_.lock())
    {
        float width = static_cast<float>(game->screen_width);
        float half_width = width*0.5f;

        game_character& player = *ptr;
        c->x = player.x - half_width;

        float right_end = static_cast<float>(game->map.width);

        if (c->x < 0.0f)
            c->x = 0.0f;
        else if (c->x + width > right_end)
            c->x = right_end - width;
    }

    return true;
}
