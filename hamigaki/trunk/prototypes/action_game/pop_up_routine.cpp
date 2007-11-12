// pop_up_routine.cpp: the pop-up routine for items

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "pop_up_routine.hpp"
#include "collision_utility.hpp"
#include "velocity_routine.hpp"

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

class pop_up_routine_impl
{
public:
    pop_up_routine_impl(float vy, int frames) : vy_(vy), frames_(frames)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        for (int i = 0; i < frames_; ++i)
        {
            boost::tie(game,c) = self.yield(true);
            c->y += vy_;
        }

        c->move_routine = &velocity_routine;
        return true;
    }

private:
    float vy_;
    int frames_;
};

} // namespace

pop_up_routine::pop_up_routine(float vy, int frames)
    : coroutine_(pop_up_routine_impl(vy, frames))
{
}

bool pop_up_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
