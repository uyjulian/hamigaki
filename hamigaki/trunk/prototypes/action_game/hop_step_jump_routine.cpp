// hop_step_jump_routine.cpp: the routine for hop-step-jump

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "hop_step_jump_routine.hpp"
#include "routine_state.hpp"

routine_result hop_step_jump_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    stat.accelerate(vx_);

    while (true)
    {
        a.ay = 0.0f;

        for (std::size_t i = 0; i < 20; ++i)
        {
            stat.yield();

            if (v.vx == 0.0f)
                stat.turn(vx_);
            else
                a.ax = 0.0f;
        }

        if (is_on_ground(map_, r))
        {
            a.ay = vy_ / 4;

            do
            {
                stat.yield();
                a.ay = 0.0f;

                if (v.vx == 0.0f)
                    stat.turn(vx_);
                else
                    a.ax = 0.0f;

            } while (!is_on_ground(map_, r));

            a.ay = vy_ / 4;

            do
            {
                stat.yield();
                a.ay = 0.0f;

                if (v.vx == 0.0f)
                    stat.turn(vx_);
                else
                    a.ax = 0.0f;

            } while (!is_on_ground(map_, r));

            a.ay = vy_;

            do
            {
                stat.yield();
                a.ay = 0.3f;

                if (v.vx == 0.0f)
                    stat.turn(vx_);
                else
                    a.ax = 0.0f;

            } while (!is_on_ground(map_, r));
        }
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}
