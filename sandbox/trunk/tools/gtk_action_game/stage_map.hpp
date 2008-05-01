// stage_map.hpp: stage map for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef STAGE_MAP_HPP
#define STAGE_MAP_HPP

#include "map_element.hpp"
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

typedef boost::multi_index::multi_index_container<
    map_element,
    boost::multi_index::indexed_by<
        boost::multi_index::ordered_unique<
            boost::multi_index::const_mem_fun<
                map_element, std::pair<int,int>, &map_element::x_y
            >
        >,
        boost::multi_index::ordered_unique<
            boost::multi_index::const_mem_fun<
                map_element, std::pair<int,int>, &map_element::y_x
            >
        >
    >
> map_elements;

struct stage_map
{
    int width;
    int height;
    std::pair<int,int> player_position;
    map_elements elements;
};

#endif // STAGE_MAP_HPP
