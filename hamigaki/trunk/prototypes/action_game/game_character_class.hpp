// game_character_class.hpp: game character class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_CHARACTER_CLASS_HPP
#define GAME_CHARACTER_CLASS_HPP

#include "char_attributes.hpp"
#include <hamigaki/uuid.hpp>
#include <bitset>
#include <string>

struct game_character_class
{
    std::string sprite;

    float width;
    float height;
    float vx;
    float vy;

    std::bitset<char_attr::max_value> attrs;
    slope_type::values slope;

    hamigaki::uuid move_routine;
    hamigaki::uuid speed_routine;
    hamigaki::uuid on_collide_block_side;
    hamigaki::uuid on_hit_from_below;
    hamigaki::uuid on_collide_player;
    hamigaki::uuid on_collide_enemy;
    hamigaki::uuid on_stomp;
    hamigaki::uuid on_hit;

    game_character_class()
        : width(32.0f), height(32.0f), vx(0.0f), vy(0.0f)
        , slope(slope_type::none)
    {
    }
};

namespace boost { namespace serialization {

template<class Archive>
inline void serialize(
    Archive& ar, game_character_class& c, const unsigned int file_version)
{
    ar & c.sprite & c.width & c.height & c.vx & c.vy;

    unsigned long attrs;
    int slope;
    if (Archive::is_saving::value)
    {
        attrs = c.attrs.to_ulong();
        slope = static_cast<int>(c.slope);
    }
    ar & attrs & slope;
    if (Archive::is_loading::value)
    {
        c.attrs = std::bitset<char_attr::max_value>(attrs);
        c.slope = static_cast<slope_type::values>(slope);
    }

    ar & c.move_routine;
    ar & c.speed_routine;
    ar & c.on_collide_block_side;
    ar & c.on_hit_from_below;
    ar & c.on_collide_player;
    ar & c.on_collide_enemy;
    ar & c.on_stomp;
    ar & c.on_hit;
}

} } // End namespaces serialization, boost.

#endif // GAME_CHARACTER_CLASS_HPP
