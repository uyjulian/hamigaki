// empty.hpp: empty() functor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/spirit for library home page.

#ifndef HAMIGAKI_SPIRIT_PHOENIX_STL_EMPTY_HPP
#define HAMIGAKI_SPIRIT_PHOENIX_STL_EMPTY_HPP

#include <boost/spirit/phoenix/functions.hpp>

namespace hamigaki { namespace phoenix {

struct empty_impl
{
    template<class T>
    struct result
    {
        typedef bool type;
    };

    template<class T>
    bool operator()(const T& c) const
    {
        return c.empty();
    }
};

const ::phoenix::function<empty_impl> empty = empty_impl();

} } // End namespaces phoenix, hamigaki.

#endif // HAMIGAKI_SPIRIT_PHOENIX_STL_EMPTY_HPP
