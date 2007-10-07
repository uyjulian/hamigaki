// game_character.hpp: game character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_CHARACTER_HPP
#define GAME_CHARACTER_HPP

#include "four_char_code.hpp"
#include "effect_base.hpp"
#include "routine_base.hpp"

class direct3d_texture9;

struct game_character
{
    rect position;
    velocity speed;
    routine_type routine;
    routine_type tmp_routine;
    effect_type effect;
    sprite_form form;
    int step;
    unsigned long color;
    const sprite_info_set* sprite_infos;
    direct3d_texture9* texture;
    std::pair<int,int> origin;
    char next_char;

    game_character() : step(0), next_char(' ')
    {
    }

    void change_form(boost::uint32_t f)
    {
        const sprite_info& old = sprite_infos->get_group(form.type)[0];
        const sprite_info& cur = sprite_infos->get_group(f)[0];
        ::change_form(position, old, cur);
        form.type = f;
        step = 0;
    }

    void move(const input_command& cmd, const stage_map& map)
    {
        acceleration a;
        sprite_form f;

        boost::tie(a, f) = routine(position, speed, form, cmd);
        if (!tmp_routine.empty())
        {
            boost::optional<routine_result> res =
                tmp_routine(std::nothrow, position, speed, form, cmd);

            if (res)
                boost::tie(a, f) = *res;
            else
                tmp_routine = routine_type();
        }

        boost::uint32_t old_form = form.type;
        form = f;

        if (form.type == sprite_form::nform)
            return;
        else if (old_form != form.type)
            change_form(form.type);
        else if (int d = static_cast<int>(std::abs(speed.vx/2.0f)))
            step += d;
        else
            ++step;

        ::move(position, speed, a, map);

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
