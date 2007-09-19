// game_character.hpp: game character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_CHARACTER_HPP
#define GAME_CHARACTER_HPP

#include "effect_base.hpp"
#include "routine_base.hpp"

class direct3d_texture9;

struct game_character
{
    move_info position;
    routine_type routine;
    effect_type effect;
    std::size_t form;
    int step;
    bool back;
    unsigned long color;
    sprite_info_set* sprite_infos;
    direct3d_texture9* texture;

    game_character() : form(0), step(0), back(false)
    {
    }

    void change_form(std::size_t f)
    {
        const sprite_info& old = sprite_infos->get_group(form)[0];
        const sprite_info& cur = sprite_infos->get_group(f)[0];
        position.change_form(old, cur);
        form = f;
        step = 0;
    }

    void move(const input_command& cmd, const stage_map& map)
    {
        acceleration a;
        std::size_t f;

        boost::tie(a, f) = routine(position, form, cmd);

        std::size_t old_form = form;
        form = f;

        if (form == sprite_info_set::nform)
            return;
        else if (old_form != form)
            change_form(form);
        else
            ++(step);

        position.move(a, map);

        if (position.vx >= 1.0f)
            back = false;
        else if (position.vx <= -1.0f)
            back = true;

        color = 0xFFFFFFFFul;
        if (!effect.empty())
        {
            boost::optional<unsigned long> res = effect(std::nothrow);
            if (res)
                color = *res;
            else
                effect = effect_type();
        }
    }
};

#endif // GAME_CHARACTER_HPP
