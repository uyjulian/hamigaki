// path_table_record.hpp: ISO 9660 path table record

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_PATH_TABLE_RECORD_HPP
#define HAMIGAKI_ARCHIVERS_ISO_PATH_TABLE_RECORD_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso {

struct path_table_record
{
    boost::uint8_t dir_id_size;
    boost::uint8_t ext_record_size;
    boost::uint32_t data_pos;
    boost::uint16_t parent_dir_number;
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::path_table_record>
{
private:
    typedef archivers::iso::path_table_record self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::dir_id_size>,
        member<self, boost::uint8_t, &self::ext_record_size>,
        member<self, boost::uint32_t, &self::data_pos, default_>,
        member<self, boost::uint16_t, &self::parent_dir_number, default_>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_PATH_TABLE_RECORD_HPP
