// turn_routine.hpp: the routine for going straight with a turn

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef TURN_ROUTINE_HPP
#define TURN_ROUTINE_HPP

struct game_character;
struct game_system;

bool turn_routine(game_system* game, game_character* c);

#endif // TURN_ROUTINE_HPP
