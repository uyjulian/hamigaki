// processor.hpp: a processor by the coroutine

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#ifndef HAMIGAKI_COROUTINE_PROCESSOR_HPP
#define HAMIGAKI_COROUTINE_PROCESSOR_HPP

#include <hamigaki/coroutine/shared_coroutine.hpp>
#include <boost/shared_ptr.hpp>
#include <iterator>

namespace hamigaki { namespace coroutines {

template<class T, class ContextImpl=detail::default_context_impl>
class processor
    : public std::iterator<
        std::output_iterator_tag, void, void, void, void
    >
{
public:
    typedef shared_coroutine1<void,T,ContextImpl> coroutine_type;
    typedef typename coroutine_type::self self;

    template <class Functor>
    explicit processor(Functor func) : coro_(func)
    {
    }

    processor& operator=(const T& value)
    {
        coro_(value);
        return *this;
    }

    processor& operator*()
    {
        return *this;
    }

    processor& operator++()
    {
        return *this;
    }

    processor operator++(int)
    {
        return *this;
    }

private:
    coroutine_type coro_;
};

} } // End namespaces coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_PROCESSOR_HPP
