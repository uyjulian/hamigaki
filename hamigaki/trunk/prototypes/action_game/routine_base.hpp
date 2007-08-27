// routine_base.hpp: base definitions for the routines

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ROUTINE_BASE_HPP
#define ROUTINE_BASE_HPP

#include "stage_map.hpp"
#include <hamigaki/coroutine/shared_coroutine.hpp>

struct rect
{
    float x;
    float y;
    float lx;
    float ly;

    int left_block() const;
    int right_block() const;
    int top_block() const;
    int bottom_block() const;
};

struct input_command
{
    float x;
    float y;
    bool jump;
    bool dash;
    bool reset;
};

typedef hamigaki::coroutines::shared_coroutine<
    rect(rect,input_command,const stage_map*)
> routine_type;

bool is_on_ground(const stage_map& map, const rect& r);
bool find_vertical_blocks(const stage_map& map, int x, int y1, int y2);
bool find_horizontal_blocks(const stage_map& map, int y, int x1, int x2);


struct move_info
{
    float x;
    float y;
    float vx;
    float vy;
};

move_info move(rect r, float vx, float vy, const stage_map& map);

#endif // ROUTINE_BASE_HPP
