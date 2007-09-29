// hop_step_jump_routine.hpp: the routine for hop-step-jump

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HOP_STEP_JUMP_ROUTINE_HPP
#define HOP_STEP_JUMP_ROUTINE_HPP

#include "routine_base.hpp"

class hop_step_jump_routine
{
public:
    explicit hop_step_jump_routine(
        const stage_map& map, float vx = 1.0f, float vy = 8.0f
    )
        : map_(map), vx_(vx), vy_(vy)
    {
    }

    routine_result operator()(
        routine_type::self& self, rect r, velocity v,
        sprite_form form, input_command cmd) const;

private:
    const stage_map& map_;
    float vx_;
    float vy_;
};

#endif // HOP_STEP_JUMP_ROUTINE_HPP
