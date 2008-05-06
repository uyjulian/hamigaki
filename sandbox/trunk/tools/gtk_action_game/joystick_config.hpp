// joystick_config.hpp: joystick configuration

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef JOYSTICK_CONFIG_HPP
#define JOYSTICK_CONFIG_HPP

#include <boost/config.hpp>

#include <boost/algorithm/string/replace.hpp>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

struct joystick_config
{
    int jump;
    int dash;
    int punch;
    int reset;

    joystick_config() : jump(0), dash(1), punch(2), reset(-1)
    {
    }
};

typedef std::pair<std::string,joystick_config> joystick_config_pair;
typedef std::vector<joystick_config_pair> joystick_config_list;

inline
void load_joystick_config_list(const char* filename, joystick_config_list& ls)
{
    std::ifstream is(filename);
    if (!is)
        return;

    joystick_config_list tmp;

    while (is)
    {
        std::string s;
        if (!(is >> s))
            break;

        std::string name = boost::algorithm::replace_all_copy(s, "%20", " ");

        joystick_config cfg;
        if (!(is >> cfg.jump))
            cfg.jump = -1;
        if (!(is >> cfg.dash))
            cfg.dash = -1;
        if (!(is >> cfg.punch))
            cfg.punch = -1;
        if (!(is >> cfg.reset))
            cfg.reset = -1;

        tmp.push_back(std::make_pair(name, cfg));
    }

    std::swap(ls, tmp);
}

inline void append_joystick_config(
    const char* filename,
    const std::string& name, const joystick_config& cfg)
{
    std::ofstream os(filename, std::ios_base::out|std::ios_base::app);
    if (!os)
        return;

    os
        << '\n'
        << boost::algorithm::replace_all_copy(name, " ", "%20") << ' '
        << cfg.jump << ' '
        << cfg.dash << ' '
        << cfg.punch << ' '
        << cfg.reset << '\n';
}

#endif // JOYSTICK_CONFIG_HPP
