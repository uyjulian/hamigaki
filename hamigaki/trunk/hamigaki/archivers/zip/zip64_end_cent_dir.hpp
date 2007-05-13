// zip64_end_cent_dir.hpp: Zip64 end of central directory

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_ZIP64_END_CENT_DIR_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_ZIP64_END_CENT_DIR_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace zip {

struct zip64_end_cent_dir
{
    static const boost::uint32_t signature = 0x06064B50;

    boost::uint64_t self_size;
    boost::uint16_t made_by;
    boost::uint16_t needed_to_extract;
    boost::uint32_t disk_number;
    boost::uint32_t start_disk_number;
    boost::uint64_t entries;
    boost::uint64_t total_entries;
    boost::uint64_t size;
    boost::uint64_t offset;
};

} } } // End namespaces zip, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::zip::zip64_end_cent_dir>
{
private:
    typedef archivers::zip::zip64_end_cent_dir self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint64_t, &self::self_size, little>,
        member<self, boost::uint16_t, &self::made_by, little>,
        member<self, boost::uint16_t, &self::needed_to_extract, little>,
        member<self, boost::uint32_t, &self::disk_number, little>,
        member<self, boost::uint32_t, &self::start_disk_number, little>,
        member<self, boost::uint64_t, &self::entries, little>,
        member<self, boost::uint64_t, &self::total_entries, little>,
        member<self, boost::uint64_t, &self::size, little>,
        member<self, boost::uint64_t, &self::offset, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ZIP_ZIP64_END_CENT_DIR_HPP
