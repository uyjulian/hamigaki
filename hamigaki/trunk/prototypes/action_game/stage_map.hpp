// stage_map.hpp: stage map for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef STAGE_MAP_HPP
#define STAGE_MAP_HPP

#include <string>
#include <utility>
#include <vector>

bool is_block(char c);

class stage_map
{
public:
    char operator()(int x, int y) const;

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

    std::pair<int,int> player_position() const;
    void replace(int x, int y, char type);

    void erase(int x, int y)
    {
        replace(x, y, ' ');
    }

private:
    std::vector<std::string> data_;
};

void load_map_from_text(const char* filename, stage_map& m);

#endif // STAGE_MAP_HPP
