// knock_back_routine.cpp: the routine for knocking back

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "knock_back_routine.hpp"

routine_result knock_back_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;

    a.ax = -v.vx + dx_;
    a.ay = -v.vy;

    boost::tie(r,v,form,cmd) = self.yield(std::make_pair(a,form));

    a.ax = 0.0f;
    a.ay = 0.0f;

    for (std::size_t i = 0; i < frames_; ++i)
        boost::tie(r,v,form,cmd) = self.yield(std::make_pair(a,form));

    form.type = sprite_form::normal;
    return std::make_pair(a,form);
}
