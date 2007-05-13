// timestamp.hpp: Windows timestamp extended header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_WINDOWS_TIMESTAMP_HPP
#define HAMIGAKI_ARCHIVERS_LHA_WINDOWS_TIMESTAMP_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace lha { namespace windows {

struct timestamp
{
    boost::uint64_t creation_time;
    boost::uint64_t last_write_time;
    boost::uint64_t last_access_time;
};

} } } } // End namespaces windows, lha, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::lha::windows::timestamp>
{
private:
    typedef archivers::lha::windows::timestamp self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint64_t, &self::creation_time, little>,
        member<self, boost::uint64_t, &self::last_write_time, little>,
        member<self, boost::uint64_t, &self::last_access_time, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_WINDOWS_TIMESTAMP_HPP
