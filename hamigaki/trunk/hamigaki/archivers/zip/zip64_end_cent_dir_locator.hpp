// zip64_end_cent_dir_locator.hpp: Zip64 end of central directory locator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_ZIP64_END_CENT_DIR_LOCATOR_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_ZIP64_END_CENT_DIR_LOCATOR_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace zip {

struct zip64_end_cent_dir_locator
{
    static const boost::uint32_t signature = 0x07064B50;

    boost::uint32_t disk_number;
    boost::uint64_t offset;
    boost::uint32_t total_disk_number;
};

} } } // End namespaces zip, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::zip::zip64_end_cent_dir_locator>
{
private:
    typedef archivers::zip::zip64_end_cent_dir_locator self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint32_t, &self::disk_number, little>,
        member<self, boost::uint64_t, &self::offset, little>,
        member<self, boost::uint32_t, &self::total_disk_number, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ZIP_ZIP64_END_CENT_DIR_LOCATOR_HPP
