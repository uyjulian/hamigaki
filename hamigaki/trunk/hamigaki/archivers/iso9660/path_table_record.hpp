//  path_table_record.hpp: ISO 9660 path table record

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO9660_PATH_TABLE_RECORD_HPP
#define HAMIGAKI_ARCHIVERS_ISO9660_PATH_TABLE_RECORD_HPP

#include <hamigaki/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace iso9660 {

struct path_table_record
{
    boost::uint8_t dir_id_size;
    boost::uint8_t ext_record_size;
    boost::uint32_t data_pos;
    boost::uint16_t parent_dir_number;
};

} } } // End namespaces iso9660, archivers, hamigaki.

namespace hamigaki
{

// TODO: to get it working for non-native endian
template<>
struct struct_traits<archivers::iso9660::path_table_record>
{
private:
    typedef archivers::iso9660::path_table_record self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::dir_id_size>,
        member<self, boost::uint8_t, &self::ext_record_size>,
        member<self, boost::uint32_t, &self::data_pos>,
        member<self, boost::uint16_t, &self::parent_dir_number>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO9660_PATH_TABLE_RECORD_HPP
