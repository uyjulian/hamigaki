// char_attribues.hpp: game character attribues

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef CHAR_ATTRIBUES_HPP
#define CHAR_ATTRIBUES_HPP

namespace char_attr
{
    enum values
    {
        player,
        enemy,
        weapon,
        block,
        breaker,

        max_value
    };
};

namespace slope_type
{
    enum values
    {
        none,
        left_down,
        right_down
    };
};

#endif // CHAR_ATTRIBUES_HPP
