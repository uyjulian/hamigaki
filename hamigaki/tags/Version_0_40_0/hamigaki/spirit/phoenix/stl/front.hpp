// front.hpp: front() functor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/spirit for library home page.

#ifndef HAMIGAKI_SPIRIT_PHOENIX_STL_FRONT_HPP
#define HAMIGAKI_SPIRIT_PHOENIX_STL_FRONT_HPP

#include <hamigaki/type_traits/member_access_traits.hpp>
#include <boost/spirit/phoenix/functions.hpp>

namespace hamigaki { namespace phoenix {

struct front_impl
{
    template<class T>
    struct result
    {
        typedef typename T::value_type value_type;
        typedef typename member_access_traits<T,value_type>::reference type;
    };

    template<class T>
    typename result<T>::type operator()(T& c) const
    {
        return *c.begin();
    }
};

const ::phoenix::function<front_impl> front = front_impl();

} } // End namespaces phoenix, hamigaki.

#endif // HAMIGAKI_SPIRIT_PHOENIX_STL_FRONT_HPP
