// side_scrolling_routine.hpp: the routine for side-scrolling camera

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SIDE_SCROLLING_ROUTINE_HPP
#define SIDE_SCROLLING_ROUTINE_HPP

#include "game_system.hpp"
#include <boost/weak_ptr.hpp>

class side_scrolling_routine
{
public:
    explicit side_scrolling_routine(const character_ptr& player);
    bool operator()(game_system* game, game_character* c) const;

private:
    boost::weak_ptr<game_character> player_;
};

#endif // SIDE_SCROLLING_ROUTINE_HPP
