// four_char_code.hpp: four-character code utilities

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef FOUR_CHAR_CODE_HPP
#define FOUR_CHAR_CODE_HPP

#include <boost/cstdint.hpp>

template<char C1, char C2, char C3, char C4>
struct static_four_char_code
{
    static const boost::uint32_t value =
        (static_cast<boost::uint32_t>(static_cast<unsigned char>(C1))      ) |
        (static_cast<boost::uint32_t>(static_cast<unsigned char>(C2)) <<  8) |
        (static_cast<boost::uint32_t>(static_cast<unsigned char>(C3)) << 16) |
        (static_cast<boost::uint32_t>(static_cast<unsigned char>(C4)) << 24) ;
};

inline boost::uint32_t four_char_code(char c1, char c2, char c3, char c4)
{
    return
        (static_cast<boost::uint32_t>(static_cast<unsigned char>(c1))      ) |
        (static_cast<boost::uint32_t>(static_cast<unsigned char>(c2)) <<  8) |
        (static_cast<boost::uint32_t>(static_cast<unsigned char>(c3)) << 16) |
        (static_cast<boost::uint32_t>(static_cast<unsigned char>(c4)) << 24) ;
}

inline boost::uint32_t four_char_code(const char* s)
{
    return four_char_code(s[0], s[1], s[2], s[3]);
}

#endif // FOUR_CHAR_CODE_HPP
