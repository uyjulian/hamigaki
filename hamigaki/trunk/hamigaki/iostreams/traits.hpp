// traits.hpp: extension of boost/iostreams/traits.hpp

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_TRAITS_HPP
#define HAMIGAKI_IOSTREAMS_TRAITS_HPP

#include <boost/iostreams/traits.hpp>

namespace hamigaki { namespace iostreams {

template<typename T>
struct is_input_seekable
    : boost::iostreams::detail::has_trait<T, boost::iostreams::input_seekable>
    {};

template<typename T>
struct is_output_seekable
    : boost::iostreams::detail::has_trait<T, boost::iostreams::output_seekable>
    {};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_TRAITS_HPP
