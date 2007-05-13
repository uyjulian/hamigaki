// sp_system_use_entry_data.hpp: IEEE P1281 "SP" System Use Entry

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_SP_SYSTEM_USE_ENTRY_DATA_HPP
#define HAMIGAKI_ARCHIVERS_ISO_SP_SYSTEM_USE_ENTRY_DATA_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso {

struct sp_system_use_entry_data
{
    char magic[2];
    boost::uint8_t skipped;
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::sp_system_use_entry_data>
{
private:
    typedef archivers::iso::sp_system_use_entry_data self;

public:
    typedef boost::mpl::list<
        member<self, char[2], &self::magic>,
        member<self, boost::uint8_t, &self::skipped>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_SP_SYSTEM_USE_ENTRY_DATA_HPP
