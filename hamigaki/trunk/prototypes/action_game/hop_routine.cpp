// hop_routine.cpp: the routine for hopping

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "hop_routine.hpp"
#include "routine_state.hpp"

namespace
{

class x_routine
{
public:
    explicit x_routine(float vx) : vx_(vx)
    {
    }

    routine_result operator()(
        routine_type::self& self,
        rect r, velocity v, sprite_form form, input_command cmd) const;

private:
    float vx_;
};

routine_result x_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    stat.accelerate(vx_);

    while (true)
    {
        stat.yield();

        if (v.vx == 0.0f)
            stat.turn(vx_);
        else
            a.ax = 0.0f;
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}

class y_routine
{
public:
    y_routine(const stage_map& map, float vy) : map_(map), vy_(vy)
    {
    }

    routine_result operator()(
        routine_type::self& self,
        rect r, velocity v, sprite_form form, input_command cmd) const;

private:
    const stage_map& map_;
    float vy_;
};

routine_result y_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    while (!is_on_ground(map_, r))
        stat.yield();

    while (true)
        stat.jump(map_, vy_, 0.3f);

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}

} // namespace

routine_result hop_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    routine_type rx((x_routine(vx_)));
    routine_type ry(y_routine(map_, vy_));

    while (true)
    {
        acceleration ax, ay;
        boost::tie(ax,form) = rx(r,v,form,cmd);
        boost::tie(ay,form) = ry(r,v,form,cmd);

        acceleration a;
        a.ax = ax.ax;
        a.ay = ay.ay;
        boost::tie(r,v,form,cmd) = self.yield(a,form);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}
