// turn_routine.cpp: the routine for going straight with a turn

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "turn_routine.hpp"
#include <cmath>

namespace
{

bool is_on_ground_left(const stage_map& map, const rect& r)
{
    int x = left_block(r);
    int y = bottom_block(r) - 1;

    return map(x, y) == '=';
}

bool is_on_ground_right(const stage_map& map, const rect& r)
{
    int x = right_block(r);
    int y = bottom_block(r) - 1;

    return map(x, y) == '=';
}

} // namespace

routine_result turn_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    a.ay = 0.0f;

    while (true)
    {
        a.ax = -v.vx;
        form.options |= sprite_options::back;
        boost::tie(r,v,form,cmd) = self.yield(a,form);

        a.ax = -1.2f;
        boost::tie(r,v,form,cmd) = self.yield(a,form);

        a.ax = 0.0f;
        while ((v.vx < 0.0f) && is_on_ground_left(map_, r))
            boost::tie(r,v,form,cmd) = self.yield(a,form);

        a.ax = -v.vx;
        form.options &= ~sprite_options::back;
        boost::tie(r,v,form,cmd) = self.yield(a,form);

        a.ax = 1.2f;
        boost::tie(r,v,form,cmd) = self.yield(a,form);

        a.ax = 0.0f;
        while ((v.vx > 0.0f) && is_on_ground_right(map_, r))
            boost::tie(r,v,form,cmd) = self.yield(a,form);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result(a,form))
}
