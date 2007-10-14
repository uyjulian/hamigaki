// pop_up_routine.cpp: the pop-up routine for items

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "pop_up_routine.hpp"
#include "routine_state.hpp"

routine_result pop_up_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    a.ay = vy_ + ay_;
    stat.yield();

    if (frames_ > 1u)
    {
        a.ay = ay_;
        stat.wait(frames_-1);
    }

    a.ay = -v.vy + ay_;
    return routine_result(a, form);
}
