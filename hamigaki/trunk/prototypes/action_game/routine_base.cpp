// routine_base.cpp: base definitions for the routines

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "routine_base.hpp"
#include <cmath>

namespace
{

float limit_abs(float x, float abs)
{
    if (std::abs(x) <= abs)
        return x;

    if (x < 0.0f)
        return -abs;
    else
        return abs;
}

} // namespace

int left_block(const rect& r)
{
    return static_cast<int>(r.x) / 32;
}

int right_block(const rect& r)
{
    if (r.lx == 0.0f)
        return left_block(r);
    else
        return static_cast<int>(std::ceil((r.x+r.lx) / 32.0f)) - 1;
}

int top_block(const rect& r)
{
    if (r.ly == 0.0f)
        return bottom_block(r);
    else
        return static_cast<int>(std::ceil((r.y+r.ly) / 32.0f)) - 1;
}

int bottom_block(const rect& r)
{
    return static_cast<int>(r.y) / 32;
}


bool is_on_ground(const stage_map& map, const rect& r)
{
    if (r.lx == 0.0f)
        return false;

    int y = bottom_block(r);
    if (r.y == static_cast<float>(y*32))
    {
        int x1 = left_block(r);
        int x2 = right_block(r);

        for (int x = x1; x <= x2; ++x)
            if (map(x, y-1) == '=')
                return true;
    }
    return false;
}

bool find_vertical_blocks(const stage_map& map, int x, int y1, int y2)
{
    for (int y = y1; y <= y2; ++y)
        if (map(x, y) == '=')
            return true;

    return false;
}

bool find_horizontal_blocks(const stage_map& map, int y, int x1, int x2)
{
    for (int x = x1; x <= x2; ++x)
        if (map(x, y) == '=')
            return true;

    return false;
}


void move_info::move(const acceleration& a, const stage_map& map)
{
    const float gravity = -0.6f;

    bool on_ground = is_on_ground(map, r);

    // FIXME
    if (r.y > -32.0f)
        vx += a.ax;
    else
        vx = 0.0f;

    if (vx < 0.0f)
    {
        int old_x = left_block(r);

        r.x += vx;
        if (r.x < 0.0f)
        {
            r.x = 0.0f;
            vx = 0.0f;
        }

        int new_x = left_block(r);
        int y1 = bottom_block(r);
        int y2 = top_block(r);

        for (int x = old_x-1; x >= new_x; --x)
        {
            if (find_vertical_blocks(map, x, y1, y2))
            {
                r.x = static_cast<float>((x+1) * 32);
                vx = 0.0f;
                break;
            }
        }
    }
    else if (vx > 0.0f)
    {
        int old_x = right_block(r);

        r.x += vx;
        float max_x = static_cast<float>(map.width()*32) - r.lx;
        if (r.x > max_x)
        {
            r.x = max_x;
            vx = 0.0f;
        }

        int new_x = right_block(r);
        int y1 = bottom_block(r);
        int y2 = top_block(r);

        for (int x = old_x+1; x <= new_x; ++x)
        {
            if (find_vertical_blocks(map, x, y1, y2))
            {
                r.x = static_cast<float>(x * 32) - r.lx;
                vx = 0.0f;
                break;
            }
        }
    }

    vy += a.ay;

    if (on_ground)
    {
        if (vy < 0.0f)
            vy = 0.0f;
    }
    else
    {
        vy += gravity;
        vy = (std::max)(vy, -10.0f);
    }

    if (vy > 0.0f)
        r.y += vy;
    else if (vy < 0.0f)
    {
        int old_y = bottom_block(r);

        r.y += vy;
        r.y = (std::max)(r.y, -64.0f); // FIXME

        if (r.lx != 0.0f)
        {
            int new_y = bottom_block(r);
            int x1 = left_block(r);
            int x2 = right_block(r);

            for (int y = old_y-1; y >= new_y; --y)
            {
                if (find_horizontal_blocks(map, y, x1, x2))
                {
                    r.y = static_cast<float>((y+1) * 32);
                    vy = 0.0f;
                    break;
                }
            }
        }
    }
}

void move_info::change_form(const sprite_info& old, const sprite_info& cur)
{
    float x = r.x - static_cast<float>(old.left);

    r.x = x + static_cast<float>(cur.left);
    r.lx = static_cast<float>(cur.width);
    r.ly = static_cast<float>(cur.height);
}
