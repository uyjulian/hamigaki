// hop_routine.cpp: the routine for hopping

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "hop_routine.hpp"

routine_result hop_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    a.ax = 0.0f;

    while (true)
    {
        if (is_on_ground(map_, r))
            a.ay = vy_;
        else
            a.ay = 0.3f;

        boost::tie(r,v,form,cmd) = self.yield(a,form);

        if (v.vx == 0.0f)
        {
            if ((form.options & sprite_options::back) != 0)
            {
                a.ax = vx_;
                form.options &= ~sprite_options::back;
            }
            else
            {
                a.ax = -vx_;
                form.options |= sprite_options::back;
            }
        }
        else
            a.ax = 0.0f;
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result(a,form))
}
