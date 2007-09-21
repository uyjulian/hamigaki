// straight_routine.hpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef STRAIGHT_ROUTINE_HPP
#define STRAIGHT_ROUTINE_HPP

#include "routine_base.hpp"

routine_result
straight_routine(
    routine_type::self& self, move_info mv,
    boost::uint32_t form, input_command cmd);

#endif // STRAIGHT_ROUTINE_HPP
