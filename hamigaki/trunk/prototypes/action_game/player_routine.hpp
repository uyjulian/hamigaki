// player_routine.hpp: the routine for player character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PLAYER_ROUTINE_HPP
#define PLAYER_ROUTINE_HPP

#include <hamigaki/coroutine/shared_coroutine.hpp>

struct game_character;
struct game_system;

class player_routine
{
public:
    player_routine();
    bool operator()(game_system* game, game_character* c) const;

private:
    hamigaki::coroutines::shared_coroutine<
        bool (game_system*, game_character*)
    > coroutine_;
};

#endif // PLAYER_ROUTINE_HPP
