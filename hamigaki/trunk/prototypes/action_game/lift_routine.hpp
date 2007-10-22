// lift_routine.hpp: the routine for lifts

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef LIFT_ROUTINE_HPP
#define LIFT_ROUTINE_HPP

#include "routine_base.hpp"

class lift_routine
{
public:
    explicit lift_routine(float speed) : speed_(speed)
    {
    }

    routine_result operator()(
        routine_type::self& self, rect r, velocity v,
        sprite_form form, input_command cmd) const;

private:
    float speed_;
};

#endif // LIFT_ROUTINE_HPP
