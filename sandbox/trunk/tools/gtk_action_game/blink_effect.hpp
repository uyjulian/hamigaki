// blink_effect.hpp: the routine for the blink effect

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef BLINK_EFFECT_HPP
#define BLINK_EFFECT_HPP

#include <hamigaki/coroutine/shared_coroutine.hpp>

struct game_character;
struct game_system;

class blink_effect
{
public:
    blink_effect();
    bool operator()(game_system* game, game_character* c) const;

private:
    hamigaki::coroutines::shared_coroutine<
        bool (game_system*, game_character*)
    > coroutine_;
};

#endif // BLINK_EFFECT_HPP
