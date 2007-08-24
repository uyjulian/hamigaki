// straight_routine.hpp: the routine for player character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef STRAIGHT_ROUTINE_HPP
#define STRAIGHT_ROUTINE_HPP

#include "routine_base.hpp"

rect straight_routine(
    routine_type::self& self, rect r, input_command cmd, const stage_map* map);

#endif // STRAIGHT_ROUTINE_HPP
