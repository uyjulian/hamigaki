// game_character_class.hpp: game character class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_CHARACTER_CLASS_HPP
#define GAME_CHARACTER_CLASS_HPP

#include "char_attributes.hpp"
#include "physics_types.hpp"
#include <hamigaki/uuid.hpp>
#include <boost/serialization/version.hpp>
#include <bitset>
#include <string>

struct game_character_class
    : private boost::totally_ordered<game_character_class>
{
    hamigaki::uuid id;
    std::string icon;
    rectangle<int> icon_rect;
    std::string sprite;

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

    bool modified;

    game_character_class()
        : vx(0.0f), vy(0.0f), slope(slope_type::none), modified(false)
    {
    }

    bool operator<(const game_character_class& rhs) const
    {
        return id < rhs.id;
    }

    bool operator==(const game_character_class& rhs) const
    {
        return id == rhs.id;
    }
};

BOOST_CLASS_VERSION(game_character_class, 1);

namespace boost { namespace serialization {

template<class Archive>
inline void serialize(
    Archive& ar, game_character_class& c, const unsigned int file_version)
{
    ar & make_nvp("id", c.id);

    if (file_version > 0)
    {
        ar & make_nvp("icon", c.icon);
        ar & make_nvp("icon-rect", c.icon_rect);
    }

    ar & make_nvp("sprite", c.sprite);
    ar & make_nvp("vx", c.vx);
    ar & make_nvp("vy", c.vy);

    unsigned long attrs;
    if (Archive::is_saving::value)
        attrs = c.attrs.to_ulong();
    ar & make_nvp("attrs", attrs);
    if (Archive::is_loading::value)
        c.attrs = std::bitset<char_attr::max_value>(attrs);

    ar & make_nvp("slope", c.slope);

    ar & make_nvp("move-routine", c.move_routine);
    ar & make_nvp("speed-routine", c.speed_routine);
    ar & make_nvp("on-collide-block-side", c.on_collide_block_side);
    ar & make_nvp("on-hit-from-below", c.on_hit_from_below);
    ar & make_nvp("on-collide-player", c.on_collide_player);
    ar & make_nvp("on-collide-enemy", c.on_collide_enemy);
    ar & make_nvp("on-stomp", c.on_stomp);
    ar & make_nvp("on-hit", c.on_hit);
}

} } // End namespaces serialization, boost.

#endif // GAME_CHARACTER_CLASS_HPP
