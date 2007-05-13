// zip_file.hpp: Phil Katz Zip file device

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ZLIB_PARAMS_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ZLIB_PARAMS_HPP

#include <hamigaki/archivers/detail/zlib.hpp>

namespace hamigaki { namespace archivers { namespace detail {

inline boost::iostreams::zlib_params make_zlib_params()
{
    boost::iostreams::zlib_params params;
    params.noheader = true;
    return params;
}

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ZLIB_PARAMS_HPP
