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
    scoped_change_z_order(game_character* c, float z)
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

class scoped_color_guard : private boost::noncopyable
{
public:
    explicit scoped_color_guard(game_character* c)
        : ptr_(c), color_(ptr_->color)
    {
    }

    ~scoped_color_guard()
    {
        ptr_->color = color_;
    }

private:
    game_character* ptr_;
    unsigned long color_;
};

class pipe_down_routine_impl
{
public:
    explicit pipe_down_routine_impl(const transfer_info& info) : info_(info)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        scoped_change_z_order z_gurad(c, 1.0f);
        c->vx = 0.0f;
        c->vy = 0.0f;
        c->change_form(sprite_form::normal);

        int count = static_cast<int>(std::ceil(c->height/2.0f));
        pop_up_routine enter(-2.0f, count);
        while (enter(game, c))
            boost::tie(game,c) = self.yield(true);

        scoped_color_guard color_guard(c);
        for (unsigned long i = 0; i < 16; ++i)
        {
            game->camera->color = (i*0x11ul) << 24;
            boost::tie(game,c) = self.yield(true);
        }

        game->next_pos = info_;

        for (unsigned long i = 0; i < 16; ++i)
        {
            game->camera->color = ((15-i)*0x11ul) << 24;
            boost::tie(game,c) = self.yield(true);
        }

        return false;
    }

private:
    transfer_info info_;
};

} // namespace

pipe_down_routine::pipe_down_routine(const transfer_info& info)
    : coroutine_(pipe_down_routine_impl(info))
{
}

bool pipe_down_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
