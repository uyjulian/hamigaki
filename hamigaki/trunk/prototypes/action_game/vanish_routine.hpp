// vanish_routine.hpp: the routine for vanishing character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef VANISH_ROUTINE_HPP
#define VANISH_ROUTINE_HPP

#include "routine_base.hpp"

class vanish_routine
{
public:
    explicit vanish_routine(std::size_t frames) : frames_(frames)
    {
    }

    routine_result operator()(
        routine_type::self& self, rect r, velocity v,
        sprite_form form, input_command cmd) const;

private:
    std::size_t frames_;
};

#endif // VANISH_ROUTINE_HPP
