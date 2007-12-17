// game_character.cpp: game character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "game_character.hpp"

game_character::game_character()
    : x(0.0f), y(0.0f), z(0.0f), width(0.0f), height(0.0f)
    , vx(0.0f), vy(0.0f)
    , slope(slope_type::none)
    , form(sprite_form::normal), back(false), removed(false), step(0)
    , color(0xFFFFFFFFul), sprite_infos(0)
{
}

rect game_character::bounds() const
{
    rect r;
    r.x = x - width*0.5f;
    r.y = y;
    r.lx = width;
    r.ly = height;
    return r;
}

rect game_character::texture_rect() const
{
    rect r;

    if (!sprite_infos)
        return r;

    r.x = x - static_cast<float>(sprite_infos->width) * 0.5f;
    r.y = y - static_cast<float>(sprite_infos->y);
    r.lx = static_cast<float>(sprite_infos->width);
    r.ly = static_cast<float>(sprite_infos->height);
    return r;
}

rect game_character::attack_rect() const
{
    rect r;

    if (!sprite_infos)
        return r;

    const sprite_group& grp = sprite_infos->get_group(form);
    std::size_t frame = step / sprite_infos->wait;
    const sprite_pattern& ptn = grp.patterns[frame];
    const rect& tr = texture_rect();

    if (back)
        r.x = x - static_cast<float>(ptn.attack_rect.x);
    else
        r.x = x + static_cast<float>(ptn.attack_rect.x);

    r.y = y + static_cast<float>(ptn.attack_rect.y);

    r.lx = static_cast<float>(ptn.attack_rect.lx);
    r.ly = static_cast<float>(ptn.attack_rect.ly);
    return r;
}

float game_character::slope_height(float dx) const
{
    if (width == 0.0f)
        return 0.0f;

    if (slope == slope_type::left_down)
        return height * (0.5f + dx/width);
    else if (slope == slope_type::right_down)
        return height * (0.5f - dx/width);
    else
        return height;
}

void game_character::change_form(boost::uint32_t f)
{
    if (sprite_infos)
    {
        const sprite_group& grp = sprite_infos->get_group(f);

        width = static_cast<float>(grp.bound_width);
        height = static_cast<float>(grp.bound_height);
    }

    form = f;
    step = 0;
}

void game_character::change_sprite(const sprite_info_set& infos)
{
    sprite_infos = &infos;

    const sprite_group& grp = sprite_infos->get_group(form);

    width = static_cast<float>(grp.bound_width);
    height = static_cast<float>(grp.bound_height);
}

void game_character::move(game_system& game)
{
    boost::uint32_t old_form = form;

    if (!move_routine.empty())
    {
        // Note:
        // This copy guarantees the lifetime until the call is completed.
        move_routine_type mv = move_routine;
        if (!mv(&game, this))
            move_routine.clear();
    }

    if (!effect.empty())
    {
        // Note:
        // This copy guarantees the lifetime until the call is completed.
        effect_type e = effect;
        if (!e(&game, this))
            effect.clear();
    }

    if (form == sprite_form::nform)
        return;
    else if (old_form != form)
        change_form(form);
    else if (int d = static_cast<int>(std::abs(vx/2.0f)))
        step += d;
    else
        ++step;

    if (!sprite_infos)
    {
        step = 0;
        return;
    }

    const sprite_group& grp = sprite_infos->get_group(form);
    std::size_t max_step = grp.patterns.size() * sprite_infos->wait;
    step %= max_step;
}
