// knock_back_routine.hpp: the routine for knocking back

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef KNOCK_BACK_ROUTINE_HPP
#define KNOCK_BACK_ROUTINE_HPP

#include "routine_base.hpp"

class knock_back_routine
{
public:
    knock_back_routine(std::size_t frames, float dx)
        : frames_(frames), dx_(dx)
    {
    }

    routine_result operator()(
        routine_type::self& self, move_info mv,
        std::size_t form, input_command cmd) const;

private:
    std::size_t frames_;
    float dx_;
};

#endif // KNOCK_BACK_ROUTINE_HPP
