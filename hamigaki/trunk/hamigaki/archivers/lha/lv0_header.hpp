// lv0_header.hpp: LZH level-0 header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_LV0_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_LHA_LV0_HEADER_HPP

#include <hamigaki/archivers/lha/compress_method.hpp>
#include <hamigaki/archivers/msdos/date_time.hpp>

namespace hamigaki { namespace archivers { namespace lha {

struct lv0_header
{
    boost::uint8_t header_size;
    boost::uint8_t header_checksum;
    compress_method method;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    msdos::date_time update_date_time;
    boost::uint16_t attributes;
};

} } } // End namespaces lha, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::lha::lv0_header>
{
private:
    typedef archivers::lha::lv0_header self;
    typedef archivers::lha::compress_method method_type;
    typedef archivers::msdos::date_time date_time_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::header_size>,
        member<self, boost::uint8_t, &self::header_checksum>,
        member<self, method_type, &self::method>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, date_time_type, &self::update_date_time>,
        member<self, boost::uint16_t, &self::attributes, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_LV0_HEADER_HPP
