// directory_record.hpp: ISO 9660 directory record

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ISO_DIRECTORY_RECORD_HPP
#define HAMIGAKI_ARCHIVERS_ISO_DIRECTORY_RECORD_HPP

#include <hamigaki/archivers/iso/date_time.hpp>

namespace hamigaki { namespace archivers { namespace iso {

struct directory_record
{
    boost::uint8_t record_size;
    boost::uint8_t ext_record_size;
    boost::uint32_t data_pos;
    boost::uint32_t data_size;
    binary_date_time recorded_time;
    boost::uint8_t flags;
    boost::uint8_t unit_size;
    boost::uint8_t interleave_gap_size;
    boost::uint16_t volume_seq_number;
    boost::uint8_t file_id_size;
};

} } } // End namespaces iso, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::iso::directory_record>
{
private:
    typedef archivers::iso::directory_record self;
    typedef archivers::iso::binary_date_time date_time;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::record_size>,
        member<self, boost::uint8_t, &self::ext_record_size>,
        member<self, boost::uint32_t, &self::data_pos, little>,
        member<self, boost::uint32_t, &self::data_pos, big>,
        member<self, boost::uint32_t, &self::data_size, little>,
        member<self, boost::uint32_t, &self::data_size, big>,
        member<self, date_time, &self::recorded_time>,
        member<self, boost::uint8_t, &self::flags>,
        member<self, boost::uint8_t, &self::unit_size>,
        member<self, boost::uint8_t, &self::interleave_gap_size>,
        member<self, boost::uint16_t, &self::volume_seq_number, little>,
        member<self, boost::uint16_t, &self::volume_seq_number, big>,
        member<self, boost::uint8_t, &self::file_id_size>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ISO_DIRECTORY_RECORD_HPP
