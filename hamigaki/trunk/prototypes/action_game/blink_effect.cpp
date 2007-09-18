// blink_effect.cpp: the routine for the blink effect

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "blink_effect.hpp"

unsigned long blink_effect(effect_type::self& self)
{
    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 5; ++j)
            self.yield(0x00000000ul);

        for (int j = 0; j < 5; ++j)
            self.yield(0xFFFFFFFFul);
    }
    return 0xFFFFFFFFul;
}
