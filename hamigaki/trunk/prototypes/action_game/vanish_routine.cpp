// vanish_routine.cpp: the routine for vanishing character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "vanish_routine.hpp"

routine_result vanish_routine::operator()(
    routine_type::self& self,
    move_info mv, std::size_t form, input_command cmd) const
{
    acceleration a;
    a.ax = 0.0f;
    a.ay = 0.0f;

    for (std::size_t i = 0; i < frames_; ++i)
        boost::tie(mv,form,cmd) = self.yield(std::make_pair(a, form));

    return std::make_pair(a, sprite_info_set::nform);
}
