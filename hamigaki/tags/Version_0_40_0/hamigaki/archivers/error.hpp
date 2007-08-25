// error.hpp: error classess for Hamigaki.Archivers

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_ERROR_HPP
#define HAMIGAKI_ARCHIVERS_ERROR_HPP

#include <boost/iostreams/detail/ios.hpp>

namespace hamigaki { namespace archivers {

class give_up_compression : public BOOST_IOSTREAMS_FAILURE
{
public:
    give_up_compression() : BOOST_IOSTREAMS_FAILURE("give up compression")
    {
    }
};

class password_incorrect : public BOOST_IOSTREAMS_FAILURE
{
public:
    password_incorrect() : BOOST_IOSTREAMS_FAILURE("password incorrect")
    {
    }
};

} } // End namespaces archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_ERROR_HPP
