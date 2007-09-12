// joystick_config.hpp: joystick configuration

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef JOYSTICK_CONFIG_HPP
#define JOYSTICK_CONFIG_HPP

#include <fstream>
#include <utility>

struct joystick_config
{
    int jump;
    int dash;
    int reset;

    joystick_config() : jump(0), dash(1), reset(-1)
    {
    }
};

inline void load_joystick_config(joystick_config& cfg, const char* filename)
{
    std::ifstream is(filename);
    if (!is)
        return;

    joystick_config tmp;
    if (!(is >> tmp.jump))
        tmp.jump = -1;
    if (!(is >> tmp.dash))
        tmp.dash = -1;
    if (!(is >> tmp.reset))
        tmp.reset = -1;

    std::swap(cfg, tmp);
}

#endif // JOYSTICK_CONFIG_HPP
