// er_system_use_entry_data.hpp: IEEE P1281 "ER" System Use Entry

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_ER_SYSTEM_USE_ENTRY_DATA_HPP
#define HAMIGAKI_ARCHIVERS_ISO_ER_SYSTEM_USE_ENTRY_DATA_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso {

struct er_system_use_entry_data
{
    boost::uint8_t id_size;
    boost::uint8_t desc_size;
    boost::uint8_t source_size;
    boost::uint8_t ex_version;
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::er_system_use_entry_data>
{
private:
    typedef archivers::iso::er_system_use_entry_data self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::id_size>,
        member<self, boost::uint8_t, &self::desc_size>,
        member<self, boost::uint8_t, &self::source_size>,
        member<self, boost::uint8_t, &self::ex_version>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_ER_SYSTEM_USE_ENTRY_DATA_HPP
