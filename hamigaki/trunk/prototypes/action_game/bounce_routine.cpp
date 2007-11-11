// bounce_routine.cpp: the routine for bouncing blocks

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "bounce_routine.hpp"
#include "collision_utility.hpp"

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    void (game_system*, game_character*)
> coroutine_type;

void bounce_routine_impl(
    coroutine_type::self& self, game_system* game, game_character* c)
{
    boost::tie(game,c) = self.yield();
    boost::tie(game,c) = self.yield();

    for (int i = 8; i >= -8; i -= 2)
    {
        c->y += static_cast<float>(i);
        boost::tie(game,c) = self.yield();
    }

    c->move_routine.clear();
    self.yield();
}

} // namespace

bounce_routine::bounce_routine() : coroutine_(&bounce_routine_impl)
{
}

void bounce_routine::operator()(game_system* game, game_character* c) const
{
    coroutine_(game, c);
}
