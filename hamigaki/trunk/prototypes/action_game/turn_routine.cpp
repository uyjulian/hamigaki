// turn_routine.cpp: the routine for going straight with a turn

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "turn_routine.hpp"
#include "routine_state.hpp"

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

bool is_on_ground_front(
    const stage_map& map, const rect& r, const sprite_form& form)
{
    if ((form.options & sprite_options::back) != 0)
        return is_on_ground_left(map, r);
    else
        return is_on_ground_right(map, r);
}

} // namespace

routine_result turn_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    while (true)
    {
        stat.stop();
        stat.yield();

        stat.accelerate(speed_);
        stat.yield();

        a.ax = 0.0f;
        while (v.vx != 0.0f)
        {
            if (!is_on_ground(map_, r))
                ;
            else if (!is_on_ground_front(map_, r, form))
                break;

            stat.yield();
        }

        stat.turn();
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}
