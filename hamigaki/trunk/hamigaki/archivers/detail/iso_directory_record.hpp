//  iso_directory_record.hpp: ISO 9660 directory record

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_RECORD_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_RECORD_HPP

#include <hamigaki/archivers/iso/date_time.hpp>
#include <boost/cstdint.hpp>
#include <string>

namespace hamigaki { namespace archivers { namespace detail {

struct iso_directory_record
{
    boost::uint32_t data_pos;
    boost::uint32_t data_size;
    iso::binary_date_time recorded_time;
    boost::uint8_t flags;
    std::string file_id;
    std::string system_use;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_DIRECTORY_RECORD_HPP
