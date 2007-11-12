// hop_step_jump_routine.cpp: the routine for hop-step-jump

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "hop_step_jump_routine.hpp"
#include "collision_utility.hpp"

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

class hop_step_jump_routine_impl
{
public:
    hop_step_jump_routine_impl(float vy, float ay) : vy_(vy), ay_(ay)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        while (true)
        {
            for (int i = 0; i < 20; ++i)
                boost::tie(game,c) = self.yield(true);

            while (!is_on_floor(*c, game->characters))
                boost::tie(game,c) = self.yield(true);

            c->vy = vy_ / 4.0f;
            while (!is_on_floor(*c, game->characters))
                boost::tie(game,c) = self.yield(true);

            c->vy = vy_ / 4.0f;
            while (!is_on_floor(*c, game->characters))
                boost::tie(game,c) = self.yield(true);

            c->vy = vy_ - ay_;
            while (!is_on_floor(*c, game->characters))
            {
                c->vy += ay_;
                boost::tie(game,c) = self.yield(true);
            }
        }

        HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(false)
    }

private:
    float vy_;
    float ay_;
};

} // namespace

hop_step_jump_routine::hop_step_jump_routine(float vy, float ay)
    : coroutine_(hop_step_jump_routine_impl(vy, ay))
{
}

bool
hop_step_jump_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
