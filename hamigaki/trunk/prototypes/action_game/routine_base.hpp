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

    int left_block() const
    {
        return static_cast<int>(x / 32.0f);
    }

    int right_block() const
    {
        return static_cast<int>((x+lx-0.1f) / 32.0f);
    }

    int top_block() const
    {
        return static_cast<int>(y / 32.0f);
    }

    int bottom_block() const
    {
        return static_cast<int>((y+ly-0.1f) / 32.0f);
    }
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

#endif // ROUTINE_BASE_HPP
