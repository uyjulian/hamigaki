// straight_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "straight_routine.hpp"
#include <cmath>

acceleration straight_routine(
    routine_type::self& self,
    move_info mv, input_command cmd, const stage_map* map)
{
    acceleration a;
    a.ax = -1.2f;
    a.ay = 0.0f;
    a.max_speed = 1.2f;

    while (true)
        boost::tie(mv, cmd, map) = self.yield(a);

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(r)
}
