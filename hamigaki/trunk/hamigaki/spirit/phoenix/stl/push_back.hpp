// push_back.hpp: push_back() functor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/spirit for library home page.

#ifndef HAMIGAKI_SPIRIT_PHOENIX_STL_PUSH_BACK_HPP
#define HAMIGAKI_SPIRIT_PHOENIX_STL_PUSH_BACK_HPP

#include <boost/spirit/phoenix/functions.hpp>

namespace hamigaki { namespace phoenix {

struct push_back_impl
{
    template<class T, class Value>
    struct result
    {
        typedef void type;
    };

    template<class T, class Value>
    void operator()(T& c, const Value& x) const
    {
        c.push_back(x);
    }
};

const ::phoenix::function<push_back_impl> push_back = push_back_impl();

} } // End namespaces phoenix, hamigaki.

#endif // HAMIGAKI_SPIRIT_PHOENIX_STL_PUSH_BACK_HPP
