// vanish_routine.cpp: the routine for vanishing character

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "vanish_routine.hpp"
#include "game_character.hpp"

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

class vanish_routine_impl
{
public:
    explicit vanish_routine_impl(int frames) : frames_(frames)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        for (int i = 0; i < frames_; ++i)
            boost::tie(game,c) = self.yield(true);

        c->y = -c->height - 128.0f; // FIXME

        return false;
    }

private:
    int frames_;
};

} // namespace

vanish_routine::vanish_routine(int frames)
    : coroutine_(vanish_routine_impl(frames))
{
}

bool vanish_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
