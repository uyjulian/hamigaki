// player_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "player_routine.hpp"
#include "collision_utility.hpp"
#include <cmath>

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

template<char C1, char C2, char C3, char C4>
struct static4cc
{
    static const boost::uint32_t value =
        static_four_char_code<C1,C2,C3,C4>::value;
};

const boost::uint32_t walk_form = static4cc<'W','A','L','K'>::value;
const boost::uint32_t jump_form = static4cc<'J','U','M','P'>::value;
const boost::uint32_t duck_form = static4cc<'D','U','C','K'>::value;
const boost::uint32_t duck_jump_form = static4cc<'D','J','M','P'>::value;
const boost::uint32_t brake_form = static4cc<'B','R','A','K'>::value;
const boost::uint32_t knock_back_form = static4cc<'K','N','O','K'>::value;
const boost::uint32_t miss_form = static4cc<'M','I','S','S'>::value;
const boost::uint32_t punch_form = static4cc<'P','U','N','C'>::value;
const boost::uint32_t duck_punch_form = static4cc<'D','P','N','C'>::value;
const boost::uint32_t slide_down_form = static4cc<'S','L','D','D'>::value;

bool is_breaking(float vx, float ax)
{
    return ((vx < 0.0f) && (ax > 0.0f)) || ((vx > 0.0f) && (ax < 0.0f));
}

void x_routine(game_system* game, game_character* c)
{
    const float brake = 0.125f;

    character_list& ls = game->characters;
    bool on_floor = is_on_floor(*c, ls);
    slope_type::values slope = current_slope(*c, ls);

    if (c->form != slide_down_form)
    {
        if (game->command.x < 0.0f)
            c->back = true;
        else if (game->command.x > 0.0f)
            c->back = false;
    }

    float max_speed = 3.0f;
    if (game->command.dash)
        max_speed = 5.0f;

    if (is_breaking(c->vx, game->command.x))
    {
        if (c->vx < 0.0f)
        {
            c->vx += brake*2.0f;
            c->vx = (std::min)(c->vx, 0.0f);
        }
        else
        {
            c->vx -= brake*2.0f;
            c->vx = (std::max)(c->vx, 0.0f);
        }

        if (on_floor && (c->form != brake_form))
        {
            c->form = brake_form;
            game->sound.play_se("brake.ogg");
        }
    }
    else if (
        on_floor && (slope != slope_type::none) &&
        (c->form == slide_down_form) )
    {
        if (c->vx < 0.0f)
        {
            c->vx -= 0.125f;
            c->vx = (std::max)(c->vx, -max_speed);
        }
        else
        {
            c->vx += 0.125f;
            c->vx = (std::min)(c->vx, max_speed);
        }
    }
    else if (game->command.x != 0.0f)
    {
        if (game->command.dash)
            c->vx += game->command.x * 0.125f;
        else
            c->vx += game->command.x * 0.1f;

        if (c->vx > max_speed)
            c->vx = max_speed;
        else if (c->vx < -max_speed)
            c->vx = -max_speed;

        if (on_floor)
        {
            c->form = walk_form;
            if (game->sound.se_filename() == "brake.ogg")
                game->sound.stop_se();
        }
    }
    else
    {
        if (c->vx < 0.0f)
        {
            c->vx += brake;
            c->vx = (std::min)(c->vx, 0.0f);
        }
        else
        {
            c->vx -= brake;
            c->vx = (std::max)(c->vx, 0.0f);
        }

        if (on_floor)
        {
            if (c->form != slide_down_form)
                c->form = sprite_form::normal;
            if (game->sound.se_filename() == "brake.ogg")
                game->sound.stop_se();
        }
    }

    if (on_floor)
    {
        if (c->form == slide_down_form)
        {
            if (c->vx == 0.0f)
                c->form = sprite_form::normal;
        }
        else
        {
            if ((game->command.x == 0.0f) && (game->command.y < 0.0f))
            {
                if (slope == slope_type::left_down)
                {
                    c->vx = -0.125f;
                    c->form = slide_down_form;
                }
                else if (slope == slope_type::right_down)
                {
                    c->vx = 0.125f;
                    c->form = slide_down_form;
                }
                else
                    c->form = duck_form;
            }
            else if (c->form == duck_form)
                c->form = sprite_form::normal;
        }
    }
    else if (c->form == walk_form)
        c->form = sprite_form::normal;
}

bool y_routine_impl(
    coroutine_type::self& self, game_system* game, game_character* c)
{
    bool old_jump = false;

    while (true)
    {
        while (!(
            is_on_floor(*c, game->characters) &&
            !old_jump && game->command.jump ) )
        {
            old_jump = game->command.jump;
            boost::tie(game,c) = self.yield(true);
        }

        if (c->form == duck_form)
            c->form = duck_jump_form;
        else
            c->form = jump_form;

        game->sound.play_se("jump.ogg");

        c->vy = 8.6f;
        if (std::abs(c->vx) > 4.0f)
            c->vy += 1.0f;

        boost::tie(game,c) = self.yield(true);

        float ay = 0.35f;
        while (!is_on_floor(*c, game->characters))
        {
            if (!game->command.jump || (c->vy < 0.0f))
                ay = 0.0f;

            c->vy += ay;
            boost::tie(game,c) = self.yield(true);
        }

        if (c->form == duck_jump_form)
            c->form = duck_form;
        else if (c->form == jump_form)
            c->form = sprite_form::normal;

        old_jump = game->command.jump;
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(false)
}

} // namespace

bool player_routine_impl(
    coroutine_type::self& self, game_system* game, game_character* c)
{
    coroutine_type y_routine(&y_routine_impl);

    int punch_frames = 0;
    boost::uint32_t form;
    bool old_punch = false;

    while (true)
    {
        bool punch_pushed = !old_punch && game->command.punch;
        old_punch = game->command.punch;

        if (punch_frames > 0)
            std::swap(form, c->form);
        else if (punch_pushed)
        {
            if ((c->form == duck_form) || (c->form == duck_jump_form))
                form = duck_punch_form;
            else
                form = punch_form;

            punch_frames = 5;
        }

        x_routine(game, c);
        y_routine(game, c);

        if (punch_frames > 0)
        {
            if (--punch_frames == 0)
            {
                if ((c->form != duck_form) &&
                    (c->form != duck_jump_form) &&
                    (c->form != jump_form) )
                {
                    form = walk_form;
                }
            }

            std::swap(form, c->form);
        }

        boost::tie(game,c) = self.yield(true);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(false)
}

player_routine::player_routine() : coroutine_(&player_routine_impl)
{
}

bool player_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
