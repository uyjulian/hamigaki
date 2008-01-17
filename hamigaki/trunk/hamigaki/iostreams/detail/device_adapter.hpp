// device_adapter.hpp: selection of device_adapter

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DETAIL_DEVICE_ADAPTER_HPP
#define HAMIGAKI_IOSTREAMS_DETAIL_DEVICE_ADAPTER_HPP

#include <boost/version.hpp>

#if BOOST_VERSION < 103500
    #include <boost/iostreams/detail/adapter/basic_adapter.hpp>
    #define HAMIGAKI_IOSTREAMS_DEVICE_ADAPTER \
        ::boost::iostreams::detail::basic_adapter
#else
    #include <boost/iostreams/detail/adapter/device_adapter.hpp>
    #define HAMIGAKI_IOSTREAMS_DEVICE_ADAPTER \
        ::boost::iostreams::detail::device_adapter
#endif

#endif // HAMIGAKI_IOSTREAMS_DETAIL_DEVICE_ADAPTER_HPP
