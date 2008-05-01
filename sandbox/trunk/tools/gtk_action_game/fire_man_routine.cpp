// fire_man_routine.cpp: the routine for player character with fire balls

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "fire_man_routine.hpp"
#include "game_system.hpp"
#include "hop_routine.hpp"
#include "player_routine.hpp"
#include "velocity_routine.hpp"
#include <boost/weak_ptr.hpp>

namespace
{

const boost::uint32_t duck_form = static_four_char_code<'D','U','C','K'>::value;

const boost::uint32_t duck_jump_form =
    static_four_char_code<'D','J','M','P'>::value;

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

void vanish(game_system* game, game_character* c, game_character* target)
{
    c->removed = true;
}

boost::weak_ptr<game_character>*
find_next_beam(boost::weak_ptr<game_character>* p, std::size_t n)
{
    for (std::size_t i = 0; i < n; ++i)
        if (p[i].expired())
            return p + i;
    return 0;
}

bool fire_man_routine_impl(
    coroutine_type::self& self, game_system* game, game_character* c)
{
    user_control_routine impl;
    bool old_dash = true;

    const std::size_t beams_size = 2;
    boost::weak_ptr<game_character> beams[beams_size];

    while (true)
    {
        bool dash_pushed = !old_dash && game->command.dash;
        old_dash = game->command.dash;

        impl(game, c);

        boost::weak_ptr<game_character>* next;
        if ( dash_pushed &&
            (c->form != duck_form) && (c->form != duck_jump_form) &&
            ((next = find_next_beam(beams, beams_size)) != 0) )
        {
            character_ptr beam(new game_character);

            const sprite_info_set& infos = game->sprites["beam.ags-yh"];
            const sprite_group& grp = infos.get_group(beam->form);

            if (c->back)
            {
                beam->x = c->x - c->width*0.5f;
                beam->vx = -6.0f;
            }
            else
            {
                beam->x = c->x + c->width*0.5f;
                beam->vx = 6.0f;
            }
            beam->y = c->y + c->height*0.5f;
            beam->z = 0.0f;
            beam->attrs.set(char_attr::weapon);
            beam->sprite_infos = &infos;
            beam->width = static_cast<float>(grp.bound_width);
            beam->height = static_cast<float>(grp.bound_height);
            beam->back = c->back;
            beam->move_routine = &velocity_routine;
            beam->speed_routine = hop_routine(6.0f, 0.0f);
            beam->on_collide_block_side = &vanish;
            beam->on_collide_enemy = &vanish;

            game->new_characters.push_back(beam);
            *next = beam;
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
