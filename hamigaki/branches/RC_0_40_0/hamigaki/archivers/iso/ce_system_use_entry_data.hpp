// ce_system_use_entry_data.hpp: IEEE P1281 "CE" System Use Entry

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_CE_SYSTEM_USE_ENTRY_DATA_HPP
#define HAMIGAKI_ARCHIVERS_ISO_CE_SYSTEM_USE_ENTRY_DATA_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso {

struct ce_system_use_entry_data
{
    boost::uint32_t next_pos;
    boost::uint32_t next_offset;
    boost::uint32_t next_size;
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::ce_system_use_entry_data>
{
private:
    typedef archivers::iso::ce_system_use_entry_data self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::next_pos, little>,
        member<self, boost::uint32_t, &self::next_pos, big>,
        member<self, boost::uint32_t, &self::next_offset, little>,
        member<self, boost::uint32_t, &self::next_offset, big>,
        member<self, boost::uint32_t, &self::next_size, little>,
        member<self, boost::uint32_t, &self::next_size, big>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_CE_SYSTEM_USE_ENTRY_DATA_HPP
