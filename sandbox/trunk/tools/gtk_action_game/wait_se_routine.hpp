// wait_se_routine.hpp: the routine for waiting SE

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef WAIT_SE_ROUTINE_HPP
#define WAIT_SE_ROUTINE_HPP

#include <hamigaki/coroutine/shared_coroutine.hpp>

struct game_character;
struct game_system;

class wait_se_routine
{
public:
    explicit wait_se_routine(const std::string& filename);
    bool operator()(game_system* game, game_character* c) const;

private:
    hamigaki::coroutines::shared_coroutine<
        bool (game_system*, game_character*)
    > coroutine_;
};

#endif // WAIT_SE_ROUTINE_HPP
