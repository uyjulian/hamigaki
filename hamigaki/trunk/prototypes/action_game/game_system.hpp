// game_system.hpp: game system objects

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_SYSTEM_HPP
#define GAME_SYSTEM_HPP

#include "game_character.hpp"
#include "input_engine.hpp"
#include "sound_engine.hpp"
#include "sprite_info_cache.hpp"
#include "stage_map.hpp"
#include <list>

typedef boost::shared_ptr<game_character> character_ptr;
typedef std::list<character_ptr> character_list;
typedef character_list::iterator character_iterator;

struct game_system
{
    explicit game_system(void* hwnd)
        : sound(hwnd)
        , screen_width(640), screen_height(480)
        , gravity(-0.6f), min_vy(-10.0f)
    {
    }

    input_command command;
    sound_engine sound;

    int screen_width;
    int screen_height;
    float gravity;
    float min_vy;

    stage_map map;
    sprite_info_cache sprites;

    character_list characters;
    character_list new_characters;
};

#endif // GAME_SYSTEM_HPP
