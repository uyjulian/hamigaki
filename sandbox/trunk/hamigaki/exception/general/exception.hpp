// exception.hpp: N2179 emulation

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

// Original
// http://www.pdimov.com/cpp/N2179/exception_ptr.cpp
// ===========================================================================>
// Copyright (c) 2007 Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// <===========================================================================

#ifndef HAMIGAKI_EXCEPTION_GENERAL_EXCEPTION_HPP
#define HAMIGAKI_EXCEPTION_GENERAL_EXCEPTION_HPP

#include <boost/shared_ptr.hpp>
#include <exception>
#include <ios>
#include <new>
#include <stdexcept>
#include <typeinfo>

namespace hamigaki
{

namespace eh_detail
{

class exception_base
{
public:
    virtual ~exception_base()
    {
    }

    void rethrow() const
    {
        do_rethrow();
    }

private:
    virtual void do_rethrow() const = 0;
};

template<class E>
class exception_impl : public exception_base
{
public:
    exception_impl()
    {
    }

    explicit exception_impl(const E& e) : e_(e)
    {
    }

private:
    E e_;

    void do_rethrow() const // virtual
    {
        throw e_;
    }
};

template<int dummy>
struct bad_alloc_storage
{
    static boost::shared_ptr<exception_base> instance;
};

template<int dummy>
boost::shared_ptr<exception_base>
bad_alloc_storage<dummy>::instance(new exception_impl<std::bad_alloc>);

} // namespace eh_detail

typedef boost::shared_ptr<eh_detail::exception_base> exception_ptr;

template<class E>
inline exception_ptr copy_exception(E e)
{
    try
    {
        return exception_ptr(new eh_detail::exception_impl<E>(e));
    }
    catch (...)
    {
        return eh_detail::bad_alloc_storage<0>::instance;
    }
}

inline exception_ptr current_exception()
{
    if (!std::uncaught_exception())
        return exception_ptr();

    try
    {
        throw;
    }

    // logic errors
    catch (const std::domain_error& e)
    {
        return copy_exception(e);
    }
    catch (const std::invalid_argument& e)
    {
        return copy_exception(e);
    }
    catch (const std::length_error& e)
    {
        return copy_exception(e);
    }
    catch (const std::out_of_range& e)
    {
        return copy_exception(e);
    }
    catch (const std::logic_error& e)
    {
        return copy_exception(e);
    }

    // runtime errors
    catch (const std::range_error& e)
    {
        return copy_exception(e);
    }
    catch (const std::overflow_error& e)
    {
        return copy_exception(e);
    }
    catch (const std::underflow_error& e)
    {
        return copy_exception(e);
    }
    catch (const std::runtime_error& e)
    {
        return copy_exception(e);
    }

    // I/O errors
    catch (const std::ios_base::failure& e)
    {
        return copy_exception(e);
    }

    // language errors
    catch (const std::bad_alloc& e)
    {
        return copy_exception(e);
    }
    catch (const std::bad_cast& e)
    {
        return copy_exception(e);
    }
    catch (const std::bad_typeid& e)
    {
        return copy_exception(e);
    }
    catch (const std::bad_exception& e)
    {
        return copy_exception(e);
    }

    // other
    catch (const std::exception& e)
    {
        return copy_exception(std::runtime_error(e.what()));
    }
    catch (...)
    {
        return copy_exception(std::runtime_error("unknown exception"));
    }
}

inline void rethrow_exception(exception_ptr p)
{
    p->rethrow();
}

} // namespace hamigaki

#endif // HAMIGAKI_EXCEPTION_GENERAL_EXCEPTION_HPP
