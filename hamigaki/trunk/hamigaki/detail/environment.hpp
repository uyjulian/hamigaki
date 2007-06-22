// environment.hpp: environment strings parser

// Copyright Takeshi Mouri 2006, 2007.
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

inline void get_environment_strings(std::map<std::string,std::string>& table)
{
#if defined(BOOST_WINDOWS)
    ::hamigaki::detail::windows::get_environment_strings(table);
#elif defined(BOOST_HAS_UNISTD_H)
    ::hamigaki::detail::posix::get_environment_strings(table);
#endif
}

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_ENVIRONMENT_HPP
