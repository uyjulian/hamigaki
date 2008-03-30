// clear.hpp: clear() functor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/spirit for library home page.

#ifndef HAMIGAKI_SPIRIT_PHOENIX_STL_CLEAR_HPP
#define HAMIGAKI_SPIRIT_PHOENIX_STL_CLEAR_HPP

#include <boost/spirit/phoenix/functions.hpp>

namespace hamigaki { namespace phoenix {

struct clear_impl
{
    template<class T>
    struct result
    {
        typedef void type;
    };

    template<class T>
    void operator()(T& c) const
    {
        c.clear();
    }
};

const ::phoenix::function<clear_impl> clear = clear_impl();

} } // End namespaces phoenix, hamigaki.

#endif // HAMIGAKI_SPIRIT_PHOENIX_STL_CLEAR_HPP
