// map_element.hpp: the element of the stage map

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MAP_ELEMENT_HPP
#define MAP_ELEMENT_HPP

#include <hamigaki/uuid.hpp>
#include <boost/operators.hpp>
#include <utility>

struct map_element : private boost::equality_comparable<map_element>
{
    int x;
    int y;
    hamigaki::uuid type;

    map_element() : x(), y()
    {
    }

    map_element(int x, int y, const hamigaki::uuid& type)
        : x(x), y(y), type(type)
    {
    }

    std::pair<int,int> x_y() const
    {
        return std::pair<int,int>(x,y);
    }

    std::pair<int,int> y_x() const
    {
        return std::pair<int,int>(y,x);
    }

    bool operator==(const map_element& rhs) const
    {
        return (x == rhs.x) && (y == rhs.y) && (type == rhs.type);
    }
};

#endif // MAP_ELEMENT_HPP
