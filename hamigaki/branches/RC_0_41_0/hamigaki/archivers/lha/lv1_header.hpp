// lv1_header.hpp: LZH level-1 header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_LV1_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_LHA_LV1_HEADER_HPP

#include <hamigaki/archivers/lha/compress_method.hpp>
#include <hamigaki/archivers/msdos/date_time.hpp>

namespace hamigaki { namespace archivers { namespace lha {

struct lv1_header
{
    boost::uint8_t header_size;
    boost::uint8_t header_checksum;
    compress_method method;
    boost::uint32_t skip_size;
    boost::uint32_t file_size;
    msdos::date_time update_date_time;
    boost::uint8_t reserved;
    boost::uint8_t level;
};

} } } // End namespaces lha, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::lha::lv1_header>
{
private:
    typedef archivers::lha::lv1_header self;
    typedef archivers::lha::compress_method method_type;
    typedef archivers::msdos::date_time date_time_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint8_t, &self::header_size>,
        member<self, boost::uint8_t, &self::header_checksum>,
        member<self, method_type, &self::method>,
        member<self, boost::uint32_t, &self::skip_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, date_time_type, &self::update_date_time>,
        member<self, boost::uint8_t, &self::reserved>,
        member<self, boost::uint8_t, &self::level>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_LV1_HEADER_HPP
