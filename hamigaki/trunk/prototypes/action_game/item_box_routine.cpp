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
#include <boost/weak_ptr.hpp>

namespace
{

typedef hamigaki::coroutines::shared_coroutine<
    bool (game_system*, game_character*)
> coroutine_type;

class item_box_routine_impl
{
public:
    item_box_routine_impl(const game_character& item) : item_(item)
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

        character_ptr item(new game_character(item_));
        int height = static_cast<int>(c->height);
        item->x = c->x;
        item->y = c->y;
        item->move_routine = pop_up_routine(1.0f, height);

        game->new_characters.push_back(item);

        boost::weak_ptr<game_character> wp(item);
        item.reset();

        while (character_ptr p = wp.lock())
        {
            if (p->move_routine)
                boost::tie(game,c) = self.yield(true);
            else
            {
                p->move_routine = &velocity_routine;
                break;
            }
        }

        c->z = org_z;

        return false;
    }

private:
    game_character item_;
};

} // namespace

item_box_routine::item_box_routine(const game_character& item)
    : coroutine_(item_box_routine_impl(item))
{
}

bool item_box_routine::operator()(game_system* game, game_character* c) const
{
    return coroutine_(game, c);
}
