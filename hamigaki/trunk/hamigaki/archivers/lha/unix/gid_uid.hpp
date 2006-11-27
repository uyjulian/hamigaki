//  gid_uid.hpp: UNIX gid/uid extended header

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_UNIX_GID_UID_HPP
#define HAMIGAKI_ARCHIVERS_LHA_UNIX_GID_UID_HPP

#include <hamigaki/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace lha { namespace unix {

struct gid_uid
{
    boost::uint16_t gid;
    boost::uint16_t uid;
};

} } } } // End namespaces unix, lha, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::lha::unix::gid_uid>
{
private:
    typedef archivers::lha::unix::gid_uid self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::gid, little>,
        member<self, boost::uint16_t, &self::uid, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_UNIX_GID_UID_HPP
