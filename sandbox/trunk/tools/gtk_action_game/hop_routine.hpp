// hop_routine.hpp: the routine for hopping

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HOP_ROUTINE_HPP
#define HOP_ROUTINE_HPP

struct game_character;
struct game_system;

class hop_routine
{
public:
    explicit hop_routine(float vy=8.0f, float ay=0.3f);
    bool operator()(game_system* game, game_character* c) const;

private:
    float vy_;
    float ay_;
};

#endif // HOP_ROUTINE_HPP
