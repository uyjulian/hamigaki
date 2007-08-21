// stage_map.hpp: stage map for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef STAGE_MAP_HPP
#define STAGE_MAP_HPP

#include <fstream>
#include <string>
#include <vector>

class stage_map
{
public:
    char operator()(std::size_t x, std::size_t y) const
    {
        if (y >= data_.size())
            return ' ';

        const std::string& line = data_[y];
        return (x < line.size()) ? line[x] : ' ';
    }

    void push_back(const std::string& line)
    {
        data_.push_back(line);
    }

    void clear()
    {
        data_.clear();
    }

    void swap(stage_map& rhs)
    {
        data_.swap(rhs.data_);
    }

private:
    std::vector<std::string> data_;
};

inline void load_map_from_text(stage_map& m, const char* filename)
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

#endif // STAGE_MAP_HPP
