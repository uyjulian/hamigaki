// environment.hpp: an utility for environment variables

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_ENVIRONMENT_HPP
#define HAMIGAKI_DETAIL_ENVIRONMENT_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/detail/windows/environment.hpp>
#elif defined(BOOST_HAS_UNISTD_H)
    #include <hamigaki/detail/posix/environment.hpp>
#else
    #error "Sorry, unsupported architecture"
#endif

namespace hamigaki { namespace detail {

#if defined(BOOST_WINDOWS)
typedef windows::environment_type environment_type;
#elif defined(BOOST_HAS_UNISTD_H)
typedef posix::environment_type environment_type;
#endif

inline void get_environment_variables(environment_type& table)
{
#if defined(BOOST_WINDOWS)
    ::hamigaki::detail::windows::get_environment_variables(table);
#elif defined(BOOST_HAS_UNISTD_H)
    ::hamigaki::detail::posix::get_environment_variables(table);
#endif
}

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_ENVIRONMENT_HPP
