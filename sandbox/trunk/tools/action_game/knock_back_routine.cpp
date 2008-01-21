// knock_back_routine.cpp: the routine for knocking back

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "knock_back_routine.hpp"
#include "game_character.hpp"

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

class knock_back_routine_impl
{
public:
    explicit knock_back_routine_impl(int frames, float dx)
        : frames_(frames), dx_(dx)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        if (c->back)
            c->x += dx_;
        else
            c->x -= dx_;

        for (int i = 0; i < frames_; ++i)
            boost::tie(game,c) = self.yield(true);

        c->change_form(sprite_form::normal);

        return false;
    }

private:
    int frames_;
    float dx_;
};

} // namespace

knock_back_routine::knock_back_routine(int frames, float dx)
    : coroutine_(knock_back_routine_impl(frames, dx))
{
}

bool knock_back_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
