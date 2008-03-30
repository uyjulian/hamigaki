// compress_method.hpp: LZH compress method

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_COMPRESS_METHOD_HPP
#define HAMIGAKI_ARCHIVERS_LHA_COMPRESS_METHOD_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/operators.hpp>
#include <cstring>

namespace hamigaki { namespace archivers { namespace lha {

struct compress_method
    : boost::equality_comparable<compress_method
    , boost::equality_comparable2<compress_method, const char*
      > >
{
    char id[5];

    compress_method()
    {
        std::memset(id, 0, 5);
    }

    explicit compress_method(const char* s)
    {
        std::memcpy(id, s, 5);
    }

    compress_method& operator=(const char* s)
    {
        std::memcpy(id, s, 5);
        return *this;
    }

    bool operator==(const compress_method& rhs) const
    {
        return std::memcmp(id, rhs.id, 5) == 0;
    }

    bool operator==(const char* rhs) const
    {
        return std::memcmp(id, rhs, 5) == 0;
    }

    bool empty() const
    {
        return id[0] == '\0';
    }
};

} } } // End namespaces lha, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::lha::compress_method>
{
private:
    typedef archivers::lha::compress_method self;

public:
    typedef boost::mpl::single_view<
        member<self, char[5], &self::id>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_LV0_HEADER_HPP
