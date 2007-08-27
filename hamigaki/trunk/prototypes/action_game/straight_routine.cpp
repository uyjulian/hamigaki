// straight_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "straight_routine.hpp"
#include <cmath>
#include <math.h>

rect straight_routine(
    routine_type::self& self, rect r, input_command cmd, const stage_map* map)
{
    float vx = -1.2f;
    float vy = 0.0f;

    while (true)
    {
        const move_info& mv = move(r, vx, vy, *map);

        r.x = mv.x;
        r.y = mv.y;
        vx = mv.vx;
        vy = mv.vy;

        boost::tie(r, cmd, map) = self.yield(r);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(r)
}
