// lv2_header.hpp: LZH level-1 header

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_LHA_LV2_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_LHA_LV2_HEADER_HPP

#include <hamigaki/archivers/lha/compress_method.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace lha {

struct lv2_header
{
    boost::uint16_t header_size;
    compress_method method;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
    boost::int32_t update_time;
    boost::uint8_t reserved;
    boost::uint8_t level;
};

struct lv2_header_rest
{
    boost::uint16_t crc16_checksum;
    char os;
    boost::uint16_t next_size;
};

} } } // End namespaces lha, archivers, hamigaki.

namespace hamigaki {

template<>
struct struct_traits<archivers::lha::lv2_header>
{
private:
    typedef archivers::lha::lv2_header self;
    typedef archivers::lha::compress_method method_type;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::header_size, little>,
        member<self, method_type, &self::method>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>,
        member<self, boost::int32_t, &self::update_time, little>,
        member<self, boost::uint8_t, &self::reserved>,
        member<self, boost::uint8_t, &self::level>
    > members;
};

template<>
struct struct_traits<archivers::lha::lv2_header_rest>
{
private:
    typedef archivers::lha::lv2_header_rest self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::crc16_checksum, little>,
        member<self, char, &self::os>,
        member<self, boost::uint16_t, &self::next_size, little>
    > members;
};

} // End namespace hamigaki.

#endif // HAMIGAKI_ARCHIVERS_LHA_LV2_HEADER_HPP
