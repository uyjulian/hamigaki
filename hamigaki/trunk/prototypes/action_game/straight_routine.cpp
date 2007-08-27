// straight_routine.cpp: the routine for going straight

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "straight_routine.hpp"
#include <cmath>
#include <math.h>

namespace
{

bool is_on_ground(const stage_map& map, const rect& r)
{
    if (r.y < 0.0f)
        return false;

    int y = r.bottom_block() + 1;
    if ((r.y + r.ly) == static_cast<float>(y*32))
    {
        int x1 = r.left_block();
        int x2 = r.right_block();

        for (int x = x1; x <= x2; ++x)
            if (map(x, y) != ' ')
                return true;
    }
    return false;
}

bool found_vertical_blocks(const stage_map& map, int x, int y1, int y2)
{
    for (int y = y1; y <= y2; ++y)
        if (map(x, y) != ' ')
            return true;

    return false;
}

bool found_horizontal_blocks(const stage_map& map, int y, int x1, int x2)
{
    for (int x = x1; x <= x2; ++x)
        if (map(x, y) != ' ')
            return true;

    return false;
}

} // namespace

rect straight_routine(
    routine_type::self& self, rect r, input_command cmd, const stage_map* map)
{
    const float gravity = 0.6f;

    float vx = -1.2f;
    float vy = 0.0f;

    while (true)
    {
        bool on_ground = is_on_ground(*map, r);

        {
            int old_x = r.left_block();

            r.x += vx;
            r.x = (std::max)(r.x, 0.0f);

            int new_x = r.left_block();
            int y1 = r.top_block();
            int y2 = r.bottom_block();

            for (int x = old_x-1; x >= new_x; --x)
            {
                if (found_vertical_blocks(*map, x, y1, y2))
                {
                    r.x = static_cast<float>((x+1) * 32);
                    vx = 0.0f;
                    break;
                }
            }
        }

        if (!on_ground)
        {
            vy += gravity;
            vy = (std::min)(vy, 10.0f);
        }

        if (vy > 0.0f)
        {
            int old_y = r.bottom_block();

            r.y += vy;
            r.y = (std::min)(r.y, 480.0f); // FIXME

            int new_y = r.bottom_block();
            int x1 = r.left_block();
            int x2 = r.right_block();

            for (int y = old_y+1; y <= new_y; ++y)
            {
                if (found_horizontal_blocks(*map, y, x1, x2))
                {
                    r.y = static_cast<float>(y * 32) - r.ly;
                    vy = 0.0f;
                    break;
                }
            }
        }

        boost::tie(r, cmd, map) = self.yield(r);
    }

    HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(r)
}
