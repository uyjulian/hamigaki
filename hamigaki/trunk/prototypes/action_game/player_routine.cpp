// player_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "player_routine.hpp"
#include <cmath>

namespace
{

bool is_breaking(float vx, float ax)
{
    return ((vx < 0.0f) && (ax > 0.0f)) || ((vx > 0.0f) && (ax < 0.0f));
}

} // namespace

routine_result player_routine::operator()(
    routine_type::self& self,
    move_info mv, sprite_form form, input_command cmd) const
{
    const float brake = 0.2f;

    bool old_jump = false;
    bool jump_boost = false;

    while (true)
    {
        bool on_ground = is_on_ground(map_, mv.r);
        bool jump_start = !old_jump && cmd.jump;

        if (cmd.x < 0.0f)
            form.options |= sprite_options::back;
        else if (cmd.x > 0.0f)
            form.options &= ~sprite_options::back;

        acceleration a;

        a.ax = 0.0f;
        a.ay = 0.0f;

        float max_speed = 3.0f;
        if (cmd.dash)
        {
            if (on_ground)
                max_speed = 8.0f;
            else
                max_speed = 5.0f;
        }

        if (on_ground && !jump_start && is_breaking(mv.vx, cmd.x))
        {
            if (mv.vx < 0.0f)
                a.ax = (std::min)(brake*2.0f, -mv.vx);
            else if (mv.vx > 0.0f)
                a.ax = -(std::min)(brake*2.0f, mv.vx);;

            if (form.type != brake_form)
            {
                form.type = brake_form;
                sound_.play_se("brake.ogg");
            }
        }
        else if (cmd.x != 0.0f)
        {
            a.ax = cmd.x * 0.25f;
            if (mv.vx < 0.0f)
                a.ax = (std::max)(a.ax, -max_speed-mv.vx);
            else
                a.ax = (std::min)(a.ax, max_speed-mv.vx);

            if (on_ground)
            {
                form.type = walk_form;
                if (sound_.se_filename() == "brake.ogg")
                    sound_.stop_se();
            }
        }
        else
        {
            if (mv.vx < 0.0f)
                a.ax = (std::min)(brake, -mv.vx);
            else if (mv.vx > 0.0f)
                a.ax = -(std::min)(brake, mv.vx);;

            if (on_ground)
            {
                form.type = sprite_form::normal;
                if (sound_.se_filename() == "brake.ogg")
                    sound_.stop_se();
            }
        }

        if (on_ground && (cmd.x == 0.0f) && (cmd.y < 0.0f))
            form.type = duck_form;
        else if ((form.type == duck_form) && on_ground)
            form.type = sprite_form::normal;

        if (!on_ground)
        {
            if (jump_boost && cmd.jump && (mv.vy > 0.0))
                a.ay = 0.35f;
            else
                jump_boost = false;

            if (form.type == walk_form)
                form.type = sprite_form::normal;
        }
        else if (jump_start)
        {
            a.ay = 8.0f;
            if (std::abs(mv.vx) > 4.0f)
                a.ay += 1.0f;
            jump_boost = true;
            if (form.type != duck_form)
                form.type = jump_form;

            sound_.play_se("jump.ogg");
        }
        else
            jump_boost = false;

        old_jump = cmd.jump;

        boost::tie(mv,form,cmd) = self.yield(std::make_pair(a,form));
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(std::make_pair(a,form))
}
