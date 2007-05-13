// volume_desc_set_terminator.hpp: ISO 9660 volume descriptor set terminator

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_VOLUME_DESC_SET_TERMINATOR_HPP
#define HAMIGAKI_ARCHIVERS_ISO_VOLUME_DESC_SET_TERMINATOR_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso {

struct volume_desc_set_terminator
{
    boost::uint8_t type;
    char std_id[5];
    boost::uint8_t version;
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::volume_desc_set_terminator>
{
private:
    typedef archivers::iso::volume_desc_set_terminator self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::type>,
        member<self, char[5], &self::std_id>,
        member<self, boost::uint8_t, &self::version>,
        padding<2041>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_VOLUME_DESC_SET_TERMINATOR_HPP
