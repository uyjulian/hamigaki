//  function2.hpp: tiny boost::function2 for Borland

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_COROUTINE_DETAIL_BORLAND_FUNCTION2_HPP
#define HAMIGAKI_COROUTINE_DETAIL_BORLAND_FUNCTION2_HPP

#include <boost/shared_ptr.hpp>

namespace hamigaki { namespace coroutines { namespace detail {

template<class R, class T1, class T2>
struct function2_base
{
    virtual ~function2_base() {}
    virtual R invoke(T1 a1, T2 a2) = 0;
};

template<class R, class T1, class T2, class F>
class function2_impl : public function2_base<R,T1,T2>
{
public:
    explicit function2_impl(F f) : func_(f)
    {
    }

    R invoke(T1 a1, T2 a2) // virtual
    {
        return func_(a1, a2);
    }

private:
    F func_;
};

template<class R, class T1, class T2>
class function2
{
public:
    template<class Functor>
    explicit function2(Functor f)
        : pimpl_(new function2_impl<R,T1,T2,Functor>(f))
    {
    }

    R operator()(T1 a1, T2 a2) const
    {
        return pimpl_->invoke(a1, a2);
    }

private:
    boost::shared_ptr<function2_base<R,T1,T2> > pimpl_;
};

} } } // End namespaces detail, coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_BORLAND_FUNCTION2_HPP
