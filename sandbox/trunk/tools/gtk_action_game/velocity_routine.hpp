// velocity_routine.hpp: the velocity-base moving routine

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef VELOCITY_ROUTINE_HPP
#define VELOCITY_ROUTINE_HPP

struct game_character;
struct game_system;

bool velocity_routine(game_system* game, game_character* c);
bool fly_routine(game_system* game, game_character* c);

#endif // VELOCITY_ROUTINE_HPP
