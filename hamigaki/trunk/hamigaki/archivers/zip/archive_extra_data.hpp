// archive_extra_data.hpp: ZIP archive extra data

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ZIP_ARCHIVE_EXTRA_DATA_HPP
#define HAMIGAKI_ARCHIVERS_ZIP_ARCHIVE_EXTRA_DATA_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/single_view.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace archivers { namespace zip {

struct archive_extra_data
{
    static const boost::uint32_t signature = 0x08064B50;

    boost::uint32_t extra_field_length;
};

} } } // End namespaces zip, archivers, hamigaki.

namespace hamigaki
{

template<>
struct struct_traits<archivers::zip::archive_extra_data>
{
private:
    typedef archivers::zip::archive_extra_data self;

public:
    typedef boost::mpl::single_view<
        member<self, boost::uint32_t, &self::extra_field_length, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_ARCHIVERS_ZIP_ARCHIVE_EXTRA_DATA_HPP
