// zip_internal_header.hpp: ZIP internal header

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ZIP_INTERNAL_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ZIP_INTERNAL_HEADER_HPP

#include <hamigaki/archivers/zip/headers.hpp>

namespace hamigaki { namespace archivers { namespace detail {

template<class Path>
struct zip_internal_header : public zip::basic_header<Path>
{
    boost::uint64_t offset;

    zip_internal_header() : offset(0)
    {
    }
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ZIP_INTERNAL_HEADER_HPP
