// pop_up_routine.hpp: the pop-up routine for items

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef POP_UP_ROUTINE_HPP
#define POP_UP_ROUTINE_HPP

#include "routine_base.hpp"

class pop_up_routine
{
public:
    pop_up_routine(float vy, float ay, std::size_t frames)
        : vy_(vy), ay_(ay), frames_(frames)
    {
    }

    routine_result operator()(
        routine_type::self& self, rect r, velocity v,
        sprite_form form, input_command cmd) const;

private:
    float vy_;
    float ay_;
    std::size_t frames_;
};

#endif // POP_UP_ROUTINE_HPP
