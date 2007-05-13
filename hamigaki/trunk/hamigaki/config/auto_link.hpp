// auto_link.hpp: wrapper for boost/config/auto_link.hpp

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <hamigaki/version.hpp>

#undef BOOST_VERSION_HPP
#undef BOOST_LIB_VERSION

#define BOOST_VERSION_HPP
#define BOOST_LIB_VERSION HAMIGAKI_LIB_VERSION

#include <boost/config/auto_link.hpp>

#undef BOOST_VERSION_HPP
#undef BOOST_VERSION
#undef BOOST_LIB_VERSION

#include <boost/version.hpp>
