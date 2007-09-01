// sprite_info.cpp: sprite infomation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "sprite_info.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

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
load_sprite_info_list_from_text(sprite_info_list& list, const char* filename)
{
    std::ifstream is(filename);
    if (!is)
    {
        std::string msg;
        msg = "cannot open file: ";
        msg += filename;
        throw std::runtime_error(msg);
    }

    sprite_info_list tmp;
    std::string line;

    if (!get_next_line(is, line))
        throw_invalid_format(filename);

    std::istringstream parser(line);

    std::string texture;
    int w, h;
    parser >> texture >> w >> h;
    tmp.texture(texture);
    tmp.width(w);
    tmp.height(h);

    while (get_next_line(is, line))
    {
        unsigned no;
        sprite_info info;

        parser
            >> no >> info.tu >> info.tv
            >> info.left >> info.top >> info.width >> info.height;

        tmp.push_back(no, info);
    }

    list.swap(tmp);
}
