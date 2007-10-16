// player_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "player_routine.hpp"
#include "routine_state.hpp"
#include <cmath>

namespace
{

const boost::uint32_t walk_form = player_routine::walk_form;
const boost::uint32_t jump_form = player_routine::jump_form;
const boost::uint32_t duck_form = player_routine::duck_form;
const boost::uint32_t duck_jump_form = player_routine::duck_jump_form;
const boost::uint32_t brake_form = player_routine::brake_form;

const boost::uint32_t punch_form =
    static_four_char_code<'P','U','N','C'>::value;

const boost::uint32_t duck_punch_form =
    static_four_char_code<'D','P','N','C'>::value;


bool is_breaking(float vx, float ax)
{
    return ((vx < 0.0f) && (ax > 0.0f)) || ((vx > 0.0f) && (ax < 0.0f));
}

float limit_accel(float a, float limit, float v)
{
    float new_v = v + a;

    if (a < 0.0f)
    {
        if (new_v < -limit)
            return -limit - v;
    }
    else
    {
        if (new_v > limit)
            return limit - v;
    }

    return a;
}

class x_routine
{
public:
    x_routine(const stage_map& map, sound_engine& sound)
        : map_(map), sound_(sound)
    {
    }

    routine_result operator()(
        routine_type::self& self,
        rect r, velocity v, sprite_form form, input_command cmd) const;

private:
    const stage_map& map_;
    sound_engine& sound_;
};

routine_result x_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    const float brake = 0.125f;

    while (true)
    {
        bool on_ground = is_on_ground(map_, r);

        if (cmd.x < 0.0f)
            form.options |= sprite_options::back;
        else if (cmd.x > 0.0f)
            form.options &= ~sprite_options::back;

        float max_speed = 3.0f;
        if (cmd.dash)
            max_speed = 5.0f;

        a.ax = 0.0f;

        if (is_breaking(v.vx, cmd.x))
        {
            if (v.vx < 0.0f)
                a.ax = brake*2.0f;
            else
                a.ax = -brake*2.0f;

            a.ax = limit_accel(a.ax, 0.0f, v.vx);

            if (on_ground && (form.type != brake_form))
            {
                form.type = brake_form;
                sound_.play_se("brake.ogg");
            }
        }
        else if (cmd.x != 0.0f)
        {
            if (cmd.dash)
                a.ax = cmd.x * 0.125f;
            else
                a.ax = cmd.x * 0.1f;

            a.ax = limit_accel(a.ax, max_speed, v.vx);

            if (on_ground)
            {
                form.type = walk_form;
                if (sound_.se_filename() == "brake.ogg")
                    sound_.stop_se();
            }
        }
        else
        {
            if (v.vx < 0.0f)
                a.ax = brake;
            else
                a.ax = -brake;

            a.ax = limit_accel(a.ax, 0.0f, v.vx);

            if (on_ground)
            {
                form.type = sprite_form::normal;
                if (sound_.se_filename() == "brake.ogg")
                    sound_.stop_se();
            }
        }

        if (on_ground)
        {
            if ((cmd.x == 0.0f) && (cmd.y < 0.0f))
                form.type = duck_form;
            else if (form.type == duck_form)
                form.type = sprite_form::normal;
        }
        else if (form.type == walk_form)
            form.type = sprite_form::normal;

        stat.yield();
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}

class y_routine
{
public:
    y_routine(const stage_map& map, sound_engine& sound)
        : map_(map), sound_(sound)
    {
    }

    routine_result operator()(
        routine_type::self& self,
        rect r, velocity v, sprite_form form, input_command cmd) const;

private:
    const stage_map& map_;
    sound_engine& sound_;
};

routine_result y_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    bool old_jump = false;

    while (true)
    {
        while (!(is_on_ground(map_, r) && !old_jump && cmd.jump))
        {
            old_jump = cmd.jump;
            stat.yield();
        }

        if (form.type == duck_form)
            form.type = duck_jump_form;
        else
            form.type = jump_form;

        sound_.play_se("jump.ogg");

        a.ay = 8.0f;
        if (std::abs(v.vx) > 4.0f)
            a.ay += 1.0f;

        stat.yield();

        a.ay = 0.35f;
        while (!is_on_ground(map_, r))
        {
            if (!cmd.jump || (v.vy < 0.0f))
                a.ay = 0.0f;

            stat.yield();
        }

        a.ay = 0.0f;

        if (form.type == duck_jump_form)
            form.type = duck_form;
        else if (form.type == jump_form)
            form.type = sprite_form::normal;

        old_jump = cmd.jump;
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}

} // namespace

routine_result player_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    routine_type rx(x_routine(map_, sound_));
    routine_type ry(y_routine(map_, sound_));

    int punch_frames = 0;
    boost::uint32_t form_type;
    bool old_punch = false;

    while (true)
    {
        bool punch_pushed = !old_punch && cmd.punch;
        old_punch = cmd.punch;

        if (punch_frames > 0)
            std::swap(form_type, form.type);
        else if (punch_pushed)
        {
            if ((form.type == duck_form) || (form.type == duck_jump_form))
                form_type = duck_punch_form;
            else
                form_type = punch_form;

            punch_frames = 5;
        }

        acceleration ax, ay;
        boost::tie(ax,form) = rx(r,v,form,cmd);
        boost::tie(ay,form) = ry(r,v,form,cmd);

        if (punch_frames > 0)
        {
            if (--punch_frames == 0)
            {
                if ((form.type != duck_form) &&
                    (form.type != duck_jump_form) &&
                    (form.type != jump_form) )
                {
                    form_type = walk_form;
                }
            }

            ax.ax = 0.0f;
            ay.ay = 0.0f;
            std::swap(form_type, form.type);
        }

        acceleration a;
        a.ax = ax.ax;
        a.ay = ay.ay;
        boost::tie(r,v,form,cmd) = self.yield(a,form);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(routine_result())
}
