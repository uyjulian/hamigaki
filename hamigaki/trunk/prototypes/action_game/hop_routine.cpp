// hop_routine.cpp: the routine for hopping

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "hop_routine.hpp"
#include "routine_state.hpp"

routine_result hop_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    stat.accelerate(vx_);

    while (true)
    {
        if (is_on_ground(map_, r))
            a.ay = vy_;
        else
            a.ay = 0.3f;

        stat.yield();

        if (v.vx == 0.0f)
            stat.turn(vx_);
        else
            a.ax = 0.0f;
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}
