// keyboard_config.hpp: keyboard configuration

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef KEYBOARD_CONFIG_HPP
#define KEYBOARD_CONFIG_HPP

#include <fstream>
#include <utility>

struct keyboard_config
{
    int jump;
    int dash;
    int punch;
    int reset;

    keyboard_config() : jump(0x2C), dash(0x2D), punch(0x2E), reset(-1)
    {
    }
};

inline keyboard_config load_keyboard_config(const char* filename)
{
    std::ifstream is(filename);

    keyboard_config cfg;
    is >> cfg.jump >> cfg.dash >> cfg.punch >> cfg.reset;
    return cfg;
}

inline
void save_keyboard_config(const char* filename, const keyboard_config& cfg)
{
    std::ofstream os(filename);
    os
        << cfg.jump << ' '
        << cfg.dash << ' '
        << cfg.punch << ' '
        << cfg.reset << '\n';
}

#endif // KEYBOARD_CONFIG_HPP
