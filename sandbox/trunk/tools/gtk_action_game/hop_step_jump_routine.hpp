// hop_step_jump_routine.hpp: the routine for hop-step-jump

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HOP_STEP_JUMP_ROUTINE_HPP
#define HOP_STEP_JUMP_ROUTINE_HPP

#include <hamigaki/coroutine/shared_coroutine.hpp>

struct game_character;
struct game_system;

class hop_step_jump_routine
{
public:
    explicit hop_step_jump_routine(float vy=8.0f, float ay=0.3f);
    bool operator()(game_system* game, game_character* c) const;

private:
    hamigaki::coroutines::shared_coroutine<
        bool (game_system*, game_character*)
    > coroutine_;
};

#endif // HOP_STEP_JUMP_ROUTINE_HPP
