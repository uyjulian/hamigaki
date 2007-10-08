// player_routine.hpp: the routine for player character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PLAYER_ROUTINE_HPP
#define PLAYER_ROUTINE_HPP

#include "four_char_code.hpp"
#include "routine_base.hpp"
#include "sound_engine.hpp"

class player_routine
{
public:
    static const boost::uint32_t walk_form =
        static_four_char_code<'W','A','L','K'>::value;

    static const boost::uint32_t jump_form =
        static_four_char_code<'J','U','M','P'>::value;

    static const boost::uint32_t duck_form =
        static_four_char_code<'D','U','C','K'>::value;

    static const boost::uint32_t duck_jump_form =
        static_four_char_code<'D','J','M','P'>::value;

    static const boost::uint32_t brake_form =
        static_four_char_code<'B','R','A','K'>::value;

    static const boost::uint32_t knock_back_form =
        static_four_char_code<'K','N','O','K'>::value;

    static const boost::uint32_t miss_form =
        static_four_char_code<'M','I','S','S'>::value;

    player_routine(const stage_map& map, sound_engine& sound)
        : map_(map), sound_(sound)
    {
    }

    routine_result operator()(
        routine_type::self& self, rect r, velocity v,
        sprite_form form, input_command cmd) const;

private:
    const stage_map& map_;
    sound_engine& sound_;
};

#endif // PLAYER_ROUTINE_HPP
