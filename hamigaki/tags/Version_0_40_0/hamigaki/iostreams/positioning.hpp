// positioning.hpp: the shortcut of <boost/iostreams/positioning.hpp>

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_POSITIONING_HPP
#define HAMIGAKI_IOSTREAMS_POSITIONING_HPP

#include <boost/iostreams/positioning.hpp>

namespace hamigaki { namespace iostreams {

typedef boost::iostreams::stream_offset stream_offset;

inline std::streampos to_position(stream_offset n)
{
    return boost::iostreams::offset_to_position(n);
}

inline stream_offset to_offset(std::streampos pos)
{
    return boost::iostreams::position_to_offset(pos);
}

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_POSITIONING_HPP
