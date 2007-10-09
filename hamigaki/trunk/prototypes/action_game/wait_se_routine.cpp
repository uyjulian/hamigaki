// vanish_routine.cpp: the routine for vanishing character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "wait_se_routine.hpp"
#include "routine_state.hpp"

wait_se_routine::wait_se_routine(
    sound_engine& sound, const std::string& filename
)
    : sound_(sound)
{
    sound_.play_se(filename);
}

routine_result wait_se_routine::operator()(
    routine_type::self& self,
    rect r, velocity v, sprite_form form, input_command cmd) const
{
    acceleration a;
    routine_state stat(self,r,v,form,cmd,a);

    while (sound_.playing_se())
        stat.yield();

    form.type = sprite_form::nform;
    return routine_result(a, form);
}
