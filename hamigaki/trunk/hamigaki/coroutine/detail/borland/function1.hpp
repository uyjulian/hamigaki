//  function1.hpp: tiny boost::function1 for Borland

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_COROUTINE_DETAIL_BORLAND_FUNCTION1_HPP
#define HAMIGAKI_COROUTINE_DETAIL_BORLAND_FUNCTION1_HPP

#include <boost/shared_ptr.hpp>

namespace hamigaki { namespace coroutine { namespace detail {

template<class R, class T1>
struct function1_base
{
    virtual ~function1_base() {}
    virtual R invoke(T1 a1) = 0;
};

template<class R, class T1, class F>
class function1_impl : public function1_base<R,T1>
{
public:
    explicit function1_impl(F f) : func_(f)
    {
    }

    R invoke(T1 a1) // virtual
    {
        return func_(a1);
    }

private:
    F func_;
};

template<class R, class T1>
class function1
{
public:
    template<class Functor>
    explicit function1(Functor f)
        : pimpl_(new function1_impl<R,T1,Functor>(f))
    {
    }

    R operator()(T1 a1) const
    {
        return pimpl_->invoke(a1);
    }

private:
    boost::shared_ptr<function1_base<R,T1> > pimpl_;
};

} } } // End namespaces detail, coroutine, hamigaki.

#endif // HAMIGAKI_COROUTINE_DETAIL_BORLAND_FUNCTION1_HPP
