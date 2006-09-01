//  generator.hpp: a generator by the coroutine

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

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

#include <hamigaki/coroutine/coroutine.hpp>
#include <boost/shared_ptr.hpp>
#include <iterator>

namespace hamigaki { namespace coroutine {

template<class T>
class generator
    : public std::iterator<
        std::input_iterator_tag, T, std::ptrdiff_t, const T, const T&
    >
{
public:
    typedef typename coroutine<T>::self self;

    generator()
    {
    }

    template <class Functor>
    explicit generator(Functor func) : coro_ptr_(new coroutine<T>(0, func))
    {
        increment();
    }

    bool operator==(const generator& rhs) const
    {
        return coro_ptr_ == rhs.coro_ptr_;
    }

    bool operator!=(const generator& rhs) const
    {
        return coro_ptr_ != rhs.coro_ptr_;
    }

    const T& operator*() const
    {
        return coro_ptr_->result();
    }

    const T* operator->() const
    {
        return &coro_ptr_->result();
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
    boost::shared_ptr<coroutine<T> > coro_ptr_;

    void increment()
    {
        coro_ptr_->yield();
        if (coro_ptr_->exited())
            coro_ptr_.reset();
    }
};

} } // End namespaces coroutine, hamigaki.

#endif // HAMIGAKI_COROUTINE_GENERATOR_HPP
