// stage_map.hpp: stage map for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef STAGE_MAP_HPP
#define STAGE_MAP_HPP

#include <hamigaki/uuid.hpp>
#include <map>
#include <utility>

typedef std::map<std::pair<int,int>,hamigaki::uuid> map_elements;

struct stage_map
{
    int width;
    int height;
    map_elements elements;
};

void load_map_from_text(const char* filename, stage_map& m);
void save_map_to_text(const char* filename, const stage_map& m);

#endif // STAGE_MAP_HPP
