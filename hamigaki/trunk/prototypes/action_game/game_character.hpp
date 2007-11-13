// game_character.hpp: game character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_CHARACTER_HPP
#define GAME_CHARACTER_HPP

#include "four_char_code.hpp"
#include "routine_base.hpp"
#include <boost/function.hpp>
#include <bitset>

namespace char_attr
{
    enum values
    {
        player,
        enemy,
        weapon,
        block,
        breaker,

        max_value
    };
};

namespace slope_type
{
    enum values
    {
        none,
        left_down,
        right_down
    };
};

struct game_character;
struct game_system;

typedef boost::function<
    bool (game_system*, game_character*)
> move_routine_type;

typedef boost::function<
    bool (game_system*, game_character*)
> speed_routine_type;

typedef boost::function<
    bool (game_system*, game_character*)
> effect_type;

typedef boost::function<
    void (game_system*, game_character*, game_character*)
> collision_event_type;

struct game_character
{
    float x;
    float y;
    float z;
    float width;
    float height;
    float vx;
    float vy;

    std::bitset<char_attr::max_value> attrs;
    slope_type::values slope;

    move_routine_type move_routine;
    speed_routine_type speed_routine;
    effect_type effect;
    collision_event_type on_collide_block_side;
    collision_event_type on_hit_from_below;
    collision_event_type on_get_by_player;

    boost::uint32_t form;
    bool back;
    int step;
    unsigned long color;
    const sprite_info_set* sprite_infos;
    std::pair<int,int> origin;

    game_character()
        : x(0.0f), y(0.0f), z(0.0f), width(0.0f), height(0.0f)
        , vx(0.0f), vy(0.0f)
        , slope(slope_type::none)
        , form(sprite_form::normal), back(false), step(0)
        , color(0xFFFFFFFFul), origin(-1,-1)
    {
    }

    rect bounds() const
    {
        rect r;
        r.x = x - width*0.5f;
        r.y = y;
        r.lx = width;
        r.ly = height;
        return r;
    }

    rect texture_rect() const
    {
        const sprite_info& info = sprite_infos->get_group(form)[0];

        rect r;
        r.x = x - sprite_infos->width() * 0.5f;
        r.y = y;
        r.lx = static_cast<float>(sprite_infos->width());
        r.ly = static_cast<float>(sprite_infos->height());
        return r;
    }

    rect attack_rect() const
    {
        const sprite_info& info = sprite_infos->get_group(form)[0];
        const rect& tr = texture_rect();

        rect r;
        if (back)
            r.x = tr.x + tr.lx - static_cast<float>(info.attack.x);
        else
            r.x = tr.x + static_cast<float>(info.attack.x);
        r.y = tr.y + sprite_infos->height() - static_cast<float>(info.attack.y);
        r.lx = static_cast<float>(info.attack.lx);
        r.ly = static_cast<float>(info.attack.ly);
        return r;
    }

    float slope_height(float dx) const
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

    void change_form(boost::uint32_t f)
    {
        const sprite_info& info = sprite_infos->get_group(f)[0];

        width = static_cast<float>(info.bounds.lx);
        height = static_cast<float>(info.bounds.ly);

        form = f;
        step = 0;
    }

    void change_sprite(const sprite_info_set& infos)
    {
        sprite_infos = &infos;

        sprite_info info = sprite_infos->get_group(form)[0];

        width = static_cast<float>(info.bounds.lx);
        height = static_cast<float>(info.bounds.ly);
    }

    void move(game_system& game)
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
    }
};

#endif // GAME_CHARACTER_HPP
