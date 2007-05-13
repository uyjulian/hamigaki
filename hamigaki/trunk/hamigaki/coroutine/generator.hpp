// generator.hpp: a generator by the coroutine

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

// Original Copyright
// ===========================================================================>
//  Copyright (c) 2006, Giovanni P. Deretta
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
// <===========================================================================

#ifndef HAMIGAKI_COROUTINE_GENERATOR_HPP
#define HAMIGAKI_COROUTINE_GENERATOR_HPP

#include <hamigaki/coroutine/shared_coroutine.hpp>
#include <iterator>

namespace hamigaki { namespace coroutines {

template<class T, class ContextImpl=detail::default_context_impl>
class generator
    : public std::iterator<
        std::input_iterator_tag, T, std::ptrdiff_t, const T*, const T&
    >
{
public:
    typedef shared_coroutine0<T,ContextImpl> coroutine_type;
    typedef typename coroutine_type::self self;

    generator()
    {
    }

    template <class Functor>
    explicit generator(Functor func) : coro_(func)
    {
        increment();
    }

    bool operator==(const generator& rhs) const
    {
        return coro_.empty() && rhs.coro_.empty();
    }

    bool operator!=(const generator& rhs) const
    {
        return !(*this == rhs);
    }

    const T& operator*() const
    {
        return t_.get();
    }

    const T* operator->() const
    {
        return t_.get_ptr();
    }

    generator& operator++()
    {
        increment();
        return *this;
    }

    generator operator++(int)
    {
        generator tmp(*this);
        increment();
        return tmp;
    }

private:
    coroutine_type coro_;
    boost::optional<T> t_;

    void increment()
    {
        t_ = coro_(std::nothrow);
        if (!t_)
            coro_ = coroutine_type();
    }
};

} } // End namespaces coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_GENERATOR_HPP
