// wait_se_routine.hpp: the routine for waiting SE

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef WAIT_SE_ROUTINE_HPP
#define WAIT_SE_ROUTINE_HPP

#include "routine_base.hpp"
#include "sound_engine.hpp"

class wait_se_routine
{
public:
    wait_se_routine(sound_engine& sound, const std::string& filename);

    routine_result operator()(
        routine_type::self& self, rect r, velocity v,
        sprite_form form, input_command cmd) const;

private:
    sound_engine& sound_;
};

#endif // WAIT_SE_ROUTINE_HPP
