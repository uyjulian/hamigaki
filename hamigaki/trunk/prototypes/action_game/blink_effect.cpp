// blink_effect.cpp: the routine for the blink effect

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "blink_effect.hpp"
#include "game_character.hpp"

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

bool blink_effect_impl(
    coroutine_type::self& self, game_system* game, game_character* c)
{
    for (int i = 0; i < 8; ++i)
    {
        c->color = 0x00000000ul;
        for (int j = 0; j < 5; ++j)
            boost::tie(game,c) = self.yield(true);

        c->color = 0xFFFFFFFFul;
        for (int j = 0; j < 5; ++j)
            boost::tie(game,c) = self.yield(true);
    }

    c->color = 0xFFFFFFFFul;
    return false;
}

} // namespace

blink_effect::blink_effect() : coroutine_(&blink_effect_impl)
{
}

bool blink_effect::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
