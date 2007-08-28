// player_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "player_routine.hpp"
#include <cmath>

acceleration player_routine(
    routine_type::self& self,
    move_info mv, input_command cmd, const stage_map* map)
{
    const float brake = 0.2f;

    bool old_jump = false;
    bool jump_boost = false;

    while (true)
    {
        if (cmd.reset)
        {
            old_jump = false;
            jump_boost = false;
        }

        bool on_ground = is_on_ground(*map, mv.r);
        bool jump_start = !old_jump && cmd.jump;

        acceleration a;

        a.ax = 0.0f;
        a.ay = 0.0f;
        a.max_speed = 3.0f;

        if (on_ground && cmd.dash)
            a.max_speed = 5.0f;

        if (cmd.x != 0.0f)
            a.ax = cmd.x * 0.25f;
        else
        {
            if (mv.vx < 0.0f)
                a.ax = (std::min)(brake, -mv.vx);
            else if (mv.vx > 0.0f)
                a.ax = -(std::min)(brake, mv.vx);;
        }

        if (!on_ground)
        {
            if (jump_boost && cmd.jump && (mv.vy < 0.0))
                a.ay = -0.35f;
            else
                jump_boost = false;
        }
        else if (jump_start)
        {
            a.ay = -8.0f;
            if (std::abs(mv.vx) > 4.0f)
                a.ay -= 1.0f;
            jump_boost = true;
        }
        else
            jump_boost = false;

        old_jump = cmd.jump;

        boost::tie(mv, cmd, map) = self.yield(a);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(r)
}
