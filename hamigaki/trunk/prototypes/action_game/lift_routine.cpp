// lift_routine.cpp: the routine for lifts

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "lift_routine.hpp"
#include "routine_state.hpp"

const float screen_height = 480.0f; // FIXME

routine_result lift_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    while (true)
    {
        a.ay = -v.vy + speed_;

        float next = r.y + speed_;
        if (next < -r.ly)
            a.ay = -v.vy + screen_height;
        else if (next > screen_height)
            a.ay = -v.vy - screen_height;

        stat.yield();
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}
