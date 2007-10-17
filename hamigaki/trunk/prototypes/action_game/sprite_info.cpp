// sprite_info.cpp: sprite infomation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "sprite_info.hpp"
#include "four_char_code.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>

namespace
{

bool get_next_line(std::istream& is, std::string& line)
{
    while (std::getline(is, line))
    {
        if (!line.empty() && (line[0] != '#'))
            return true;
    }
    return false;
}

void throw_invalid_format(const char* filename)
{
    std::string msg;
    msg = "invalid format: ";
    msg += filename;
    throw std::runtime_error(msg);
}

} // namespace

void
load_sprite_info_set_from_text(const char* filename, sprite_info_set& infos)
{
    std::ifstream is(filename);
    if (!is)
    {
        std::string msg;
        msg = "cannot open file: ";
        msg += filename;
        throw std::runtime_error(msg);
    }

    sprite_info_set tmp;
    std::string line;

    if (!get_next_line(is, line))
        throw_invalid_format(filename);

    std::istringstream parser(line);

    std::string texture;
    int w, h;
    parser >> texture >> w >> h;
    if (!parser)
        throw_invalid_format(filename);

    tmp.texture(texture);
    tmp.width(w);
    tmp.height(h);

    while (get_next_line(is, line))
    {
        char fcc[5];
        sprite_info info;

        parser.str(line);
        parser.clear();
        parser
            >> std::setw(5) >> fcc >> info.x >> info.y
            >> info.bounds.x >> info.bounds.y
            >> info.bounds.lx >> info.bounds.ly;

        if (!parser)
            throw_invalid_format(filename);

        parser
            >> info.attack.x >> info.attack.y
            >> info.attack.lx >> info.attack.ly;

        if (!parser)
        {
            info.attack.x = 0;
            info.attack.y = 0;
            info.attack.lx = 0;
            info.attack.ly = 0;
        }

        tmp.push_back(four_char_code(fcc), info);
    }

    infos.swap(tmp);
}
