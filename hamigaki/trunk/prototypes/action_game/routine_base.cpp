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


float slope_height(const stage_map& map, float x, float y)
{
    int x_blk = static_cast<int>(x) / 32;
    int y_blk = static_cast<int>(y) / 32;

    char c = map(x_blk, y_blk);
    if (c == '/')
    {
        float dx = x - x_blk * 32;
        return static_cast<float>(y_blk * 32) + dx;
    }
    else if (c == '\\')
    {
        float dx = x - x_blk * 32;
        return static_cast<float>(y_blk * 32 + 32) - dx;
    }
    else if (is_block(c))
        return static_cast<float>(y_blk * 32 + 32);

    c = map(x_blk, ++y_blk);
    if (c == '/')
    {
        float dx = x - x_blk * 32;
        return static_cast<float>(y_blk * 32) + dx;
    }
    else if (c == '\\')
    {
        float dx = x - x_blk * 32;
        return static_cast<float>(y_blk * 32 + 32) - dx;
    }
    --y_blk;

    c = map(x_blk, --y_blk);
    if (c == '/')
    {
        float dx = x - x_blk * 32;
        return static_cast<float>(y_blk * 32) + dx;
    }
    else if (c == '\\')
    {
        float dx = x - x_blk * 32;
        return static_cast<float>(y_blk * 32 + 32) - dx;
    }
    else
        return 0.0f;
}

bool is_slope(const stage_map& map, const rect& r)
{
    if (r.lx == 0.0f)
        return false;

    int y = bottom_block(r);

    float center = r.x + r.lx * 0.5f;
    int x = static_cast<int>(center)/32;
    char c = map(x, y);
    if ((c == '/') || (c == '\\'))
        return true;

    float dy = r.y - static_cast<float>(y*32);
    if (dy < 16.0f)
        --y;
    else
        ++y;

    c = map(x, y);
    if ((c == '/') || (c == '\\'))
        return true;

    return false;
}

bool is_on_slope(const stage_map& map, const rect& r)
{
    if (r.lx == 0.0f)
        return false;

    float center = r.x + r.lx * 0.5f;
    return slope_height(map, center, r.y) == r.y;
}

bool is_on_ground(const stage_map& map, const rect& r)
{
    if (r.lx == 0.0f)
        return false;

    if (is_slope(map, r))
        return is_on_slope(map, r);

    int y = bottom_block(r);
    if (r.y == static_cast<float>(y*32))
    {
        int x1 = left_block(r);
        int x2 = right_block(r);

        for (int x = x1; x <= x2; ++x)
            if (is_block(map(x, y-1)))
                return true;
    }
    return false;
}

bool is_in_blocks(const stage_map& map, const rect& r)
{
    if ((r.lx == 0.0f) || (r.ly == 0.0f))
        return false;

    int y1 = bottom_block(r);
    int y2 = top_block(r);

    if (is_slope(map, r))
        ++y1;

    int x1 = left_block(r);
    int x2 = right_block(r);

    for (int y = y1; y <= y2; ++y)
        for (int x = x1; x <= x2; ++x)
            if (is_block(map(x, y)))
                return true;

    return false;
}

bool find_left_wall(const stage_map& map, int x, int y1, int y2)
{
    for (int y = y1; y <= y2; ++y)
        if (is_left_wall(map(x, y)))
            return true;

    return false;
}

bool find_right_wall(const stage_map& map, int x, int y1, int y2)
{
    for (int y = y1; y <= y2; ++y)
        if (is_right_wall(map(x, y)))
            return true;

    return false;
}

bool find_floor(const stage_map& map, int y, int x1, int x2)
{
    for (int x = x1; x <= x2; ++x)
        if (is_floor(map(x, y)))
            return true;

    return false;
}

bool find_ceiling(const stage_map& map, int y, int x1, int x2)
{
    for (int x = x1; x <= x2; ++x)
        if (is_ceiling(map(x, y)))
            return true;

    return false;
}


void move(rect& r, velocity& v, const acceleration& a, const stage_map& map)
{
    bool slope = is_slope(map, r);
    bool on_ground = is_on_ground(map, r);

    v.vx += a.ax;

    if (v.vx < 0.0f)
    {
        int old_x = left_block(r);

        r.x += v.vx;

        int new_x = left_block(r);
        int y1 = bottom_block(r);
        int y2 = top_block(r);

        if (slope)
            ++y1;

        for (int x = old_x-1; x >= new_x; --x)
        {
            if (find_left_wall(map, x, y1, y2))
            {
                r.x = static_cast<float>((x+1) * 32);
                v.vx = 0.0f;
                break;
            }
        }
    }
    else if (v.vx > 0.0f)
    {
        int old_x = right_block(r);

        r.x += v.vx;

        int new_x = right_block(r);
        int y1 = bottom_block(r);
        int y2 = top_block(r);

        if (slope)
            ++y1;

        for (int x = old_x+1; x <= new_x; ++x)
        {
            if (find_right_wall(map, x, y1, y2))
            {
                r.x = static_cast<float>(x * 32) - r.lx;
                v.vx = 0.0f;
                break;
            }
        }
    }

    v.vy += a.ay;

    if (on_ground)
    {
        if (v.vy < 0.0f)
            v.vy = 0.0f;
    }

    if (v.vy > 0.0f)
        r.y += v.vy;
    else if ((v.vy < 0.0f) && !slope)
    {
        int old_y = bottom_block(r);

        r.y += v.vy;
        r.y = (std::max)(r.y, -64.0f); // FIXME

        if (r.lx != 0.0f)
        {
            int new_y = bottom_block(r);
            int x1 = left_block(r);
            int x2 = right_block(r);

            for (int y = old_y-1; y >= new_y; --y)
            {
                if (find_floor(map, y, x1, x2))
                {
                    r.y = static_cast<float>((y+1) * 32);
                    v.vy = 0.0f;
                    break;
                }
            }
        }
    }

    if (is_slope(map, r))
    {
        float center = r.x + r.lx * 0.5f;
        float h = slope_height(map, center, r.y);
        if (h > r.y)
        {
            v.vx = limit_abs(v.vx, 3.0f); // FIXME
            r.y = h;
        }
        else if (v.vy <= 0.0f)
            r.y = h;
    }
    else if (slope && (v.vy <= 0.0f))
        r.y = static_cast<float>((static_cast<int>(r.y) + 16) / 32 * 32);
}

void change_form(rect& r, const sprite_info& old, const sprite_info& cur)
{
    float x = r.x - static_cast<float>(old.bounds.x);

    r.x = x + static_cast<float>(cur.bounds.x);
    r.lx = static_cast<float>(cur.bounds.lx);
    r.ly = static_cast<float>(cur.bounds.ly);
}
