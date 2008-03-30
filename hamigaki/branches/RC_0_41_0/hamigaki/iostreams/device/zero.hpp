// zero.hpp: zero device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DEVICE_ZERO_HPP
#define HAMIGAKI_IOSTREAMS_DEVICE_ZERO_HPP

#include <hamigaki/iostreams/catable.hpp>
#include <hamigaki/iostreams/blocking.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <algorithm>

namespace hamigaki { namespace iostreams {

template<typename Ch>
class basic_zero_source
{
public:
    typedef Ch char_type;

    struct category
        : public boost::iostreams::input
        , public boost::iostreams::device_tag
        , public boost::iostreams::closable_tag
    {};

    std::streamsize read(Ch* s, std::streamsize n)
    {
        if (n <= 0)
            return -1;
        std::fill_n(s, n, Ch());
        return n;
    }

    std::streampos seek(boost::iostreams::stream_offset, BOOST_IOS::seekdir)
    {
        return -1;
    }

    void close() {}
};

typedef basic_zero_source<char>     zero_source;
typedef basic_zero_source<wchar_t>  wzero_source;

} } // End namespaces iostreams, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::iostreams::basic_zero_source, 1)
HAMIGAKI_IOSTREAMS_BLOCKING(hamigaki::iostreams::basic_zero_source, 1)

#endif // HAMIGAKI_IOSTREAMS_DEVICE_ZERO_HPP
