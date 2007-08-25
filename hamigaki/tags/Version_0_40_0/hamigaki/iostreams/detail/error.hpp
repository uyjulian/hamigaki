// error.hpp: error classess for Hamigaki.Iostreams

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_DETAIL_ERROR_HPP
#define HAMIGAKI_IOSTREAMS_DETAIL_ERROR_HPP

#include <boost/iostreams/detail/ios.hpp>

namespace hamigaki { namespace iostreams {

class out_of_restriction : public BOOST_IOSTREAMS_FAILURE
{
public:
    out_of_restriction() : BOOST_IOSTREAMS_FAILURE("out of restriction")
    {
    }

    explicit out_of_restriction(const std::string& msg)
        : BOOST_IOSTREAMS_FAILURE(msg)
    {
    }
};

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DETAIL_ERROR_HPP
