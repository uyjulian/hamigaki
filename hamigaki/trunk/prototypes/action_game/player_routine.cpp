// player_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "player_routine.hpp"
#include <cmath>
#include <math.h>

namespace
{

float limit_abs(float x, float abs)
{
    if (std::abs(x) <= abs)
        return x;

    if (x < 0.0f)
        return -abs;
    else
        return abs;
}

} // namespace

rect player_routine(
    routine_type::self& self, rect r, input_command cmd, const stage_map* map)
{
    const float brake = 0.2f;

    float vx = 0.0f;
    float vy = 0.0f;
    bool old_jump = false;
    bool jump_boost = false;

    while (true)
    {
        if (cmd.reset)
        {
            vx = 0.0f;
            vy = 0.0f;
            old_jump = false;
            jump_boost = false;
        }

        bool on_ground = is_on_ground(*map, r);
        bool jump_start = !old_jump && cmd.jump;

        float max_speed = 3.0f;
        if (on_ground && cmd.dash)
            max_speed = 5.0f;

        if (cmd.x != 0.0f)
        {
            vx += cmd.x * 0.25f;
            vx = limit_abs(vx, max_speed);
        }
        else
        {
            if (vx < 0.0f)
            {
                vx += brake;
                vx = (std::min)(vx, 0.0f);
            }
            else if (vx > 0.0f)
            {
                vx -= brake;
                vx = (std::max)(vx, 0.0f);
            }
        }

        if (!on_ground)
        {
            if (jump_boost && cmd.jump && (vy < 0.0))
                vy -= 0.35f;
            else
                jump_boost = false;
        }
        else if (jump_start)
        {
            vy = -8.0f;
            if (std::abs(vx) > 4.0f)
                vy += -1.0f;
            jump_boost = true;
        }
        else
            jump_boost = false;

        const move_info& mv = move(r, vx, vy, *map);

        r.x = mv.x;
        r.y = mv.y;
        vx = mv.vx;
        vy = mv.vy;

        old_jump = cmd.jump;

        boost::tie(r, cmd, map) = self.yield(r);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(r)
}
