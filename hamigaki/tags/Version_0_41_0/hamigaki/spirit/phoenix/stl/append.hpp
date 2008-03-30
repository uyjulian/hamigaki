// append.hpp: append() functor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/spirit for library home page.

#ifndef HAMIGAKI_SPIRIT_PHOENIX_STL_APPEND_HPP
#define HAMIGAKI_SPIRIT_PHOENIX_STL_APPEND_HPP

#include <boost/spirit/phoenix/functions.hpp>

namespace hamigaki { namespace phoenix {

struct append_impl
{
    template<class T, class U>
    struct result
    {
        typedef T& type;
    };

    template<class T, class U>
    T& operator()(T& c, const U& x) const
    {
        c.insert(c.end(), x.begin(), x.end());
        return c;
    }
};

const ::phoenix::function<append_impl> append = append_impl();

} } // End namespaces phoenix, hamigaki.

#endif // HAMIGAKI_SPIRIT_PHOENIX_STL_APPEND_HPP
