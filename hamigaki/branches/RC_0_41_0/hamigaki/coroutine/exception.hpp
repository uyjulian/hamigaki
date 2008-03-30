// exception.hpp: the exception classes for Hamigaki.Coroutine

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

#ifndef HAMIGAKI_COROUTINE_EXCEPTION_HPP
#define HAMIGAKI_COROUTINE_EXCEPTION_HPP

#include <exception>
#include <typeinfo>

namespace hamigaki { namespace coroutines {

// Note:
// The class exit_exception is NOT derived from std::exception.
// The reasons are following:
// 1. Some debuggers catch std::exception.
// 2. This exception is never caught by users.
class exit_exception {};

class exception_base : public std::exception
{
public:
    ~exception_base() throw() // virtual
    {
    }
};

class coroutine_exited : public exception_base
{
public:
    ~coroutine_exited() throw() // virtual
    {
    }

    const char* what() const throw() // virtual
    {
        return "coroutine exited";
    }
};

// Added for the compatibility to Boost.Coroutine
// Thanks for the report by W.Dee
class unknown_exception_tag {};
class abnormal_exit : public std::exception
{
public:
    explicit abnormal_exit(const std::type_info* p = 0)
        : type_ptr_(p)
    {
    }

    ~abnormal_exit() throw() // virtual
    {
    }

    const char* what() const throw() // virtual
    {
        if (type_ptr_)
            return type_ptr_->name();
        else
            return "unknown exception";
    }

    const std::type_info& type() const
    {
        if (type_ptr_)
            return *type_ptr_;
        else
            return typeid(unknown_exception_tag);
    }

    const std::type_info* type_ptr() const
    {
        return type_ptr_;
    }

private:
    const std::type_info* type_ptr_;
};

} } // End namespaces coroutines, hamigaki.

#endif // HAMIGAKI_COROUTINE_EXCEPTION_HPP
