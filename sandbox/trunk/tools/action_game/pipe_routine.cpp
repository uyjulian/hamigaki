// pipe_routine.cpp: the routine for pipes

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "pipe_routine.hpp"
#include "game_character.hpp"
#include "game_system.hpp"
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

void auto_move(
    coroutine_type::self& self, game_character* c, transfer_info::direction dir)
{
    int count = 0;
    float vx = 0.0f;
    float vy = 0.0f;

    if (dir == transfer_info::left)
    {
        count = static_cast<int>(std::ceil(c->width/2.0f));
        vx = -2.0f;
        c->back = true;
    }
    else if (dir == transfer_info::right)
    {
        count = static_cast<int>(std::ceil(c->width/2.0f));
        vx = 2.0f;
        c->back = false;
    }
    else if (dir == transfer_info::down)
    {
        count = static_cast<int>(std::ceil(c->height/2.0f));
        vy = -2.0f;
    }
    else if (dir == transfer_info::up)
    {
        count = static_cast<int>(std::ceil(c->height/2.0f));
        vy = 2.0f;
    }

    for (int i = 0; i < count; ++i)
    {
        c->x += vx;
        c->y += vy;
        self.yield(true);
    }
}

class pipe_routine_impl
{
public:
    explicit pipe_routine_impl(const transfer_info& info) : info_(info)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        scoped_change_z_order z_gurad(c, 1.0f);
        c->vx = 0.0f;
        c->vy = 0.0f;

        game->sound.play_se("pipe.ogg");

        auto_move(self, c, info_.enter_dir);

        scoped_color_guard color_guard(c);
        for (unsigned long i = 0; i < 16; ++i)
        {
            game->camera->color = (i*0x11ul) << 24;
            boost::tie(game,c) = self.yield(true);
        }

        while (game->sound.playing_se())
            boost::tie(game,c) = self.yield(true);

        game->next_pos = info_;

        for (unsigned long i = 0; i < 16; ++i)
        {
            game->camera->color = ((15-i)*0x11ul) << 24;
            boost::tie(game,c) = self.yield(true);
        }

        if (info_.leave_dir != transfer_info::none)
            game->sound.play_se("pipe.ogg");

        auto_move(self, c, info_.leave_dir);

        return false;
    }

private:
    transfer_info info_;
};

} // namespace

pipe_routine::pipe_routine(const transfer_info& info)
    : coroutine_(pipe_routine_impl(info))
{
}

bool pipe_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
