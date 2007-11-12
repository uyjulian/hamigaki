// hop_routine.cpp: the routine for hopping

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "hop_routine.hpp"
#include "collision_utility.hpp"

hop_routine::hop_routine(float vy, float ay) : vy_(vy), ay_(ay)
{
}

bool hop_routine::operator()(game_system* game, game_character* c) const
{
    if (is_on_floor(*c, game->characters))
        c->vy = vy_;
    else
        c->vy += ay_;

    return true;
}
