// stage_map.cpp: stage map for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.


#include "stage_map.hpp"
#include <fstream>

bool is_block(char c)
{
    return (c == '=') || (c == 'm') || (c == '$') || (c == 'G') || (c == '_');
}

char stage_map::operator()(int x, int y) const
{
    if (x < 0)
        return ' ';
    else if (y < 0)
        return ' ';

    std::size_t ux = static_cast<std::size_t>(x);
    std::size_t uy = static_cast<std::size_t>(y);

    if (uy >= data_.size())
        return ' ';

    const std::string& line = data_[data_.size()-1-uy];
    return (ux < line.size()) ? line[ux] : ' ';
}

std::pair<int,int> stage_map::player_position() const
{
    for (std::size_t i = 0; i < data_.size(); ++i)
    {
        const std::string& line = data_[i];
        std::string::size_type pos = line.find('@');
        if (pos != std::string::npos)
        {
            return std::make_pair(
                static_cast<int>(pos),
                static_cast<int>(data_.size() - 1 - i)
            );
        }
    }

    return std::pair<int,int>(1, 1);
}

void stage_map::replace(int x, int y, char type)
{
    if (x < 0)
        return;
    else if (y < 0)
        return;

    std::size_t ux = static_cast<std::size_t>(x);
    std::size_t uy = static_cast<std::size_t>(y);

    if (uy >= data_.size())
        return;

    std::string& line = data_[data_.size()-1-uy];
    if (ux < line.size())
        line[ux] = type;
}

void load_map_from_text(const char* filename, stage_map& m)
{
    std::ifstream is(filename);
    if (!is)
    {
        m.clear();
        return;
    }

    stage_map tmp;
    std::string line;
    while (std::getline(is, line))
    {
        if (line.empty() || (line[0] == '#'))
            continue;
        else
            tmp.push_back(line);
    }

    m.swap(tmp);
}
