// end_of_central_directory.hpp: ZIP end of central directory

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_END_OF_CENTRAL_DIRECTORY_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_END_OF_CENTRAL_DIRECTORY_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace zip {

struct end_of_central_directory
{
    static const boost::uint32_t signature = 0x06054B50;

    boost::uint16_t disk_number;
    boost::uint16_t start_disk_number;
    boost::uint16_t entries;
    boost::uint16_t total_entries;
    boost::uint32_t size;
    boost::uint32_t offset;
    boost::uint16_t comment_length;
};

} } } // End namespaces zip, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::zip::end_of_central_directory>
{
private:
    typedef archivers::zip::end_of_central_directory self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::disk_number, little>,
        member<self, boost::uint16_t, &self::start_disk_number, little>,
        member<self, boost::uint16_t, &self::entries, little>,
        member<self, boost::uint16_t, &self::total_entries, little>,
        member<self, boost::uint32_t, &self::size, little>,
        member<self, boost::uint32_t, &self::offset, little>,
        member<self, boost::uint16_t, &self::comment_length, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ZIP_END_OF_CENTRAL_DIRECTORY_HPP
