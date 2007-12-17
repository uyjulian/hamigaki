// game_character.hpp: game character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_CHARACTER_HPP
#define GAME_CHARACTER_HPP

#include "char_attributes.hpp"
#include "four_char_code.hpp"
#include "map_element.hpp"
#include "physics_types.hpp"
#include "sprite_form.hpp"
#include "sprite_info.hpp"
#include <boost/function.hpp>
#include <bitset>
#include <cmath>
#include <utility>

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
    collision_event_type on_collide_player;
    collision_event_type on_collide_enemy;
    collision_event_type on_stomp;
    collision_event_type on_hit;

    boost::uint32_t form;
    bool back;
    bool removed;
    int step;
    unsigned long color;
    const sprite_info_set* sprite_infos;
    map_element origin;

    game_character();
    rect bounds() const;
    rect texture_rect() const;
    rect attack_rect() const;
    float slope_height(float dx) const;
    void change_form(boost::uint32_t f);
    void change_sprite(const sprite_info_set& infos);
    void move(game_system& game);
};

#endif // GAME_CHARACTER_HPP
