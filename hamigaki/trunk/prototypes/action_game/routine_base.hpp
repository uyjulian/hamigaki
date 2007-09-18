// routine_base.hpp: base definitions for the routines

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ROUTINE_BASE_HPP
#define ROUTINE_BASE_HPP

#include "input_engine.hpp"
#include "sprite_info.hpp"
#include "stage_map.hpp"
#include <hamigaki/coroutine/shared_coroutine.hpp>
#include <utility>

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

inline bool includes_point(const rect& r, float x, float y)
{
    return (r.x <= x) && (r.x+r.lx > x) && (r.y <= y) && (r.y+r.ly > y);
}

inline bool intersects_rect(const rect& r1, const rect& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y < r2.y + r2.ly) && (r2.y < r1.y + r1.ly) ;
}

bool is_on_ground(const stage_map& map, const rect& r);
bool find_vertical_blocks(const stage_map& map, int x, int y1, int y2);
bool find_horizontal_blocks(const stage_map& map, int y, int x1, int x2);

struct acceleration
{
    float ax;
    float ay;
};

struct move_info
{
    rect r;
    float vx;
    float vy;

    void move(const acceleration& a, const stage_map& map);
    void change_form(const sprite_info& old, const sprite_info& cur);
};

typedef std::pair<acceleration,std::size_t> routine_result;

typedef hamigaki::coroutines::shared_coroutine<
    routine_result(move_info, std::size_t, input_command)
> routine_type;

#endif // ROUTINE_BASE_HPP
