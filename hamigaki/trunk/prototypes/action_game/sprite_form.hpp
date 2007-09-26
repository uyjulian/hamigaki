// sprite_form.hpp: sprite form infomation

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SPRITE_FORM_HPP
#define SPRITE_FORM_HPP

#include "four_char_code.hpp"

struct sprite_options
{
    static const boost::uint32_t back           = 0x00000001;
    static const boost::uint32_t upside_down    = 0x00000002;
};

struct sprite_form
{
    static const boost::uint32_t normal =
        static_four_char_code<'N','O','R','M'>::value;

    static const boost::uint32_t nform = static_cast<boost::uint32_t>(-1);

    boost::uint32_t type;
    boost::uint32_t options;

    sprite_form() : type(normal), options(0)
    {
    }
};

#endif // SPRITE_FORM_HPP
