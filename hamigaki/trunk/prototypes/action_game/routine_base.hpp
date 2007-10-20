// routine_base.hpp: base definitions for the routines

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef ROUTINE_BASE_HPP
#define ROUTINE_BASE_HPP

#include "input_engine.hpp"
#include "physics_types.hpp"
#include "sprite_form.hpp"
#include "sprite_info.hpp"
#include "stage_map.hpp"
#include <hamigaki/coroutine/shared_coroutine.hpp>
#include <utility>

int left_block(const rect& r);
int right_block(const rect& r);
int top_block(const rect& r);
int bottom_block(const rect& r);

inline bool includes_point(const rect& r, float x, float y)
{
    return (r.x <= x) && (r.x+r.lx > x) && (r.y <= y) && (r.y+r.ly > y);
}

inline bool intersect_rects(const rect& r1, const rect& r2)
{
    return
        (r1.x < r2.x + r2.lx) && (r2.x < r1.x + r1.lx) &&
        (r1.y < r2.y + r2.ly) && (r2.y < r1.y + r1.ly) ;
}

float slope_height(const stage_map& map, float x, float y);
bool is_slope(const stage_map& map, const rect& r);
bool is_on_slope(const stage_map& map, const rect& r);
bool is_on_ground(const stage_map& map, const rect& r);
bool is_in_blocks(const stage_map& map, const rect& r);
bool find_ceiling(const stage_map& map, int y, int x1, int x2);

void move(rect& r, velocity& v, const acceleration& a, const stage_map& map);
void change_form(rect& r, const sprite_info& old, const sprite_info& cur);

typedef std::pair<acceleration,sprite_form> routine_result;

typedef hamigaki::coroutines::shared_coroutine<
    routine_result(rect, velocity, sprite_form, input_command)
> routine_type;

#endif // ROUTINE_BASE_HPP
