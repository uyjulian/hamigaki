// turn_routine.hpp: the routine for going straight with a turn

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef TURN_ROUTINE_HPP
#define TURN_ROUTINE_HPP

#include "routine_base.hpp"

class turn_routine
{
public:
    explicit turn_routine(const stage_map& map) : map_(map)
    {
    }

    routine_result operator()(
        routine_type::self& self, rect r, velocity v,
        sprite_form form, input_command cmd) const;

private:
    const stage_map& map_;
};

#endif // TURN_ROUTINE_HPP
