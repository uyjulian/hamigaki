// pipe_routine.hpp: the routine for pipes

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef PIPE_ROUTINE_HPP
#define PIPE_ROUTINE_HPP

#include "transfer_info.hpp"
#include <hamigaki/coroutine/shared_coroutine.hpp>

struct game_character;
struct game_system;

class pipe_routine
{
public:
    explicit pipe_routine(const transfer_info& info);
    bool operator()(game_system* game, game_character* c) const;

private:
    hamigaki::coroutines::shared_coroutine<
        bool (game_system*, game_character*)
    > coroutine_;
};

#endif // PIPE_ROUTINE_HPP
