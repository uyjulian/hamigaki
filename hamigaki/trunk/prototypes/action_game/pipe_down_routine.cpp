// pipe_down_routine.cpp: the routine for down pipes

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "pipe_down_routine.hpp"
#include "game_character.hpp"
#include "game_system.hpp"
#include "pop_up_routine.hpp"
#include <cmath>

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

class scoped_change_z_order : private boost::noncopyable
{
public:
    explicit scoped_change_z_order(game_character* c, float z)
        : ptr_(c), z_(ptr_->z)
    {
        ptr_->z = z;
    }

    ~scoped_change_z_order()
    {
        ptr_->z = z_;
    }

private:
    game_character* ptr_;
    float z_;
};

class pipe_down_routine_impl
{
public:
    pipe_down_routine_impl(float x, float y) : x_(x), y_(y)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        scoped_change_z_order z_gurad(c, 1.0f);
        c->vx = 0.0f;
        c->vy = 0.0f;

        int count = static_cast<int>(std::ceil(c->height/2.0f));
        pop_up_routine enter(-2.0f, count);
        while (enter(game, c))
            boost::tie(game,c) = self.yield(true);

        c->x = x_;
        c->y = y_;

        // FIXME: HACK for the first move()
        c->vy = -game->gravity;

        return false;
    }

private:
    float x_;
    float y_;
};

} // namespace

pipe_down_routine::pipe_down_routine(float x, float y)
    : coroutine_(pipe_down_routine_impl(x, y))
{
}

bool pipe_down_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
