// straight_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "straight_routine.hpp"
#include <cmath>

routine_result straight_routine(
    routine_type::self& self,
    move_info mv, sprite_form form, input_command cmd)
{
    acceleration a;
    a.ay = 0.0f;

    while (true)
    {
        a.ax = -1.2f;
        form.options |= sprite_options::back;
        boost::tie(mv,form,cmd) = self.yield(std::make_pair(a,form));

        a.ax = 0.0f;
        while (mv.vx < 0.0f)
            boost::tie(mv,form,cmd) = self.yield(std::make_pair(a,form));

        a.ax = 1.2f;
        form.options &= ~sprite_options::back;
        boost::tie(mv,form,cmd) = self.yield(std::make_pair(a,form));

        a.ax = 0.0f;
        while (mv.vx > 0.0f)
            boost::tie(mv,form,cmd) = self.yield(std::make_pair(a,form));
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(std::make_pair(a,form))
}
