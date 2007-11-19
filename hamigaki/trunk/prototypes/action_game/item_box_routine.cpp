// item_box_routine.cpp: the routine for the item boxes

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "item_box_routine.hpp"
#include "bounce_routine.hpp"
#include "collision_utility.hpp"
#include "pop_up_routine.hpp"
#include "velocity_routine.hpp"

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

class item_box_routine_impl
{
public:
    item_box_routine_impl(const character_ptr& item) : item_(item)
    {
    }

    bool operator()(
        coroutine_type::self& self, game_system* game, game_character* c) const
    {
        float org_z = c->z;
        c->z = 0.0f;

        bounce_routine bounce;
        while (bounce(game, c))
            boost::tie(game,c) = self.yield(true);

        int height = static_cast<int>(c->height);
        item_->x = c->x;
        item_->y = c->y;
        item_->move_routine = pop_up_routine(1.0f, height);

        game->new_characters.push_back(item_);

        for (int i = 0; i < height; ++i)
            boost::tie(game,c) = self.yield(true);

        c->z = org_z;

        return false;
    }

private:
    character_ptr item_;
};

} // namespace

item_box_routine::item_box_routine(const character_ptr& item)
    : coroutine_(item_box_routine_impl(item))
{
}

bool item_box_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
