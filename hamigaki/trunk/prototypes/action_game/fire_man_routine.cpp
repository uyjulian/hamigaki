// fire_man_routine.cpp: the routine for player character with fire balls

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "fire_man_routine.hpp"
#include "game_system.hpp"
#include "player_routine.hpp"
#include "velocity_routine.hpp"

namespace
{

const boost::uint32_t punch_form=static_four_char_code<'P','U','N','C'>::value;

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

void vanish(game_system* game, game_character* c, game_character* target)
{
    c->removed = true;
}

bool fire_man_routine_impl(
    coroutine_type::self& self, game_system* game, game_character* c)
{
    user_control_routine impl;
    bool old_punch = false;

    while (true)
    {
        bool punch_pushed = !old_punch && game->command.punch;
        old_punch = game->command.punch;

        impl(game, c);

        if (punch_pushed && (c->form == punch_form))
        {
            game_character beam;

            const sprite_info_set& infos = game->sprites["beam.txt"];
            const sprite_info& info = infos.get_group(beam.form)[0];

            if (c->back)
            {
                beam.x = c->x - c->width*0.5f;
                beam.vx = -10.0f;
            }
            else
            {
                beam.x = c->x + c->width*0.5f;
                beam.vx = 10.0f;
            }
            beam.y = c->y + c->height*0.5f;
            beam.z = 0.0f;
            beam.attrs.set(char_attr::weapon);
            beam.sprite_infos = &infos;
            beam.width = static_cast<float>(info.bounds.lx);
            beam.height = static_cast<float>(info.bounds.ly);
            beam.back = c->back;
            beam.move_routine = &fly_routine;
            beam.on_collide_block_side = &vanish;

            game->new_characters.push_back(beam);
        }

        boost::tie(game,c) = self.yield(true);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(false)
}

} // namespace


fire_man_routine::fire_man_routine()
    : coroutine_(&fire_man_routine_impl)
{
}

bool
fire_man_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
