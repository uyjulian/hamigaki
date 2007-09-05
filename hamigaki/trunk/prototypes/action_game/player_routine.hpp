// player_routine.hpp: the routine for player character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PLAYER_ROUTINE_HPP
#define PLAYER_ROUTINE_HPP

#include "routine_base.hpp"
#include "sound_engine.hpp"

class player_routine
{
public:
    player_routine(const stage_map& map, sound_engine& sound)
        : map_(map), sound_(sound)
    {
    }

    routine_result operator()(
        routine_type::self& self, move_info mv, input_command cmd) const;

private:
    const stage_map& map_;
    sound_engine& sound_;
};

#endif // PLAYER_ROUTINE_HPP
