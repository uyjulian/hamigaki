// data_descriptor.hpp: ZIP data descriptor

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_DATA_DESCRIPTOR_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_DATA_DESCRIPTOR_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace zip {

struct data_descriptor
{
    static const boost::uint32_t signature = 0x08074B50;

    boost::uint32_t crc32_checksum;
    boost::uint32_t compressed_size;
    boost::uint32_t file_size;
};

} } } // End namespaces zip, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::zip::data_descriptor>
{
private:
    typedef archivers::zip::data_descriptor self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::crc32_checksum, little>,
        member<self, boost::uint32_t, &self::compressed_size, little>,
        member<self, boost::uint32_t, &self::file_size, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ZIP_DATA_DESCRIPTOR_HPP
