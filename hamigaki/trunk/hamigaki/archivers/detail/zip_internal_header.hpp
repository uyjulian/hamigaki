//  zip_internal_header.hpp: ZIP internal header

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ZIP_INTERNAL_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ZIP_INTERNAL_HEADER_HPP

#include <hamigaki/archivers/zip/headers.hpp>
#include <boost/iostreams/positioning.hpp>

namespace hamigaki { namespace archivers { namespace detail {

struct zip_internal_header : public zip::header
{
    boost::iostreams::stream_offset offset;

    zip_internal_header() : offset(-1)
    {
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ZIP_INTERNAL_HEADER_HPP
