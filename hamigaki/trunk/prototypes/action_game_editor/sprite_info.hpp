// sprite_info.hpp: sprite information for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SPRITE_INFO_HPP
#define SPRITE_INFO_HPP

#include "physics_types.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/cstdint.hpp>
#include <map>
#include <string>
#include <vector>

struct sprite_pattern
{
    int x;
    int y;
    rectangle<int> attack_rect;
    rectangle<int> stomp_rect;

    sprite_pattern() : x(0), y(0)
    {
    }
};

struct sprite_group
{
    int bound_width;
    int bound_height;
    std::vector<sprite_pattern> patterns;

    sprite_group() : bound_width(0), bound_height(0)
    {
    }
};

struct sprite_info_set
{
    std::string texture;
    int width;
    int height;
    int y;
    int wait;
    std::map<boost::uint32_t,sprite_group> groups;

    sprite_info_set() : width(0), height(0), y(0), wait(1)
    {
    }

    void swap(sprite_info_set& rhs)
    {
        texture.swap(rhs.texture);
        std::swap(width, rhs.width);
        std::swap(height, rhs.height);
        std::swap(y, rhs.y);
        std::swap(wait, rhs.wait);
        groups.swap(rhs.groups);
    }
};

void save_sprite_info_set(const char* filename, const sprite_info_set& infos);
void load_sprite_info_set(const char* filename, sprite_info_set& infos);
void
load_sprite_info_set_from_text(const char* filename, sprite_info_set& infos);

namespace boost { namespace serialization {

template<class Archive>
inline void serialize(
    Archive& ar, sprite_pattern& p, const unsigned int file_version)
{
    ar & make_nvp("x", p.x);
    ar & make_nvp("y", p.y);
    ar & make_nvp("attack-rect", p.attack_rect);
    ar & make_nvp("stomp-rect", p.stomp_rect);
}

template<class Archive>
inline void serialize(
    Archive& ar, sprite_group& g, const unsigned int file_version)
{
    ar & make_nvp("bound-width", g.bound_width);
    ar & make_nvp("bound-height", g.bound_height);
    ar & make_nvp("patterns", g.patterns);
}

template<class Archive>
inline void serialize(
    Archive& ar, sprite_info_set& s, const unsigned int file_version)
{
    ar & make_nvp("texture", s.texture);
    ar & make_nvp("width", s.width);
    ar & make_nvp("height", s.height);
    ar & make_nvp("y", s.y);
    ar & make_nvp("wait", s.wait);
    ar & make_nvp("groups", s.groups);
}

} } // End namespaces serialization, boost.

#endif // SPRITE_INFO_HPP
