// gid_uid.hpp: UNIX gid/uid extended header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_UNIX_GID_UID_HPP
#define HAMIGAKI_ARCHIVERS_LHA_UNIX_GID_UID_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

// Note:
// This extended header is used by LHa for UNIX. It's not a POSIX feature.
// But "unix" is the pre-defined macro on some compilers.
// So "posix" is selected for the namespaec.
namespace hamigaki { namespace archivers { namespace lha { namespace posix {

struct gid_uid
{
    boost::uint16_t gid;
    boost::uint16_t uid;
};

} } } } // End namespaces posix, lha, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::lha::posix::gid_uid>
{
private:
    typedef archivers::lha::posix::gid_uid self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::gid, little>,
        member<self, boost::uint16_t, &self::uid, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_UNIX_GID_UID_HPP
