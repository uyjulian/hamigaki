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
#include <utility>
#include <vector>

class stage_map
{
public:
    char operator()(int x, int y) const
    {
        if (x < 0)
            return '=';
        else if (y < 0)
            return ' ';

        std::size_t ux = static_cast<std::size_t>(x);
        std::size_t uy = static_cast<std::size_t>(y);

        if (uy >= data_.size())
            return ' ';

        const std::string& line = data_[data_.size()-1-uy];
        return (ux < line.size()) ? line[ux] : ' ';
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

    int width() const
    {
        return static_cast<int>(data_[0].size());
    }

    int height() const
    {
        return static_cast<int>(data_.size());
    }

    std::pair<int,int> player_position() const
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

private:
    std::vector<std::string> data_;
};

inline void load_map_from_text(const char* filename, stage_map& m)
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
