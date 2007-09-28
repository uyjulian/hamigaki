// knock_back_routine.cpp: the routine for knocking back

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "knock_back_routine.hpp"
#include "routine_state.hpp"

routine_result knock_back_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);
    stat.go_backward(dx_);
    stat.wait(frames_);

    form.type = sprite_form::normal;
    return routine_result(a, form);
}
