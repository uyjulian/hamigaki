// copy_failed_test.cpp: test case for the copy failed of an exception

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <boost/config.hpp>
#include <hamigaki/exception.hpp>
#include <boost/test/unit_test.hpp>

#if defined(BOOST_MSVC)
    #pragma warning(disable : 4290)
#endif

namespace ut = boost::unit_test;

bool allocatable = true;

void* operator new[](std::size_t size) throw(std::bad_alloc)
{
    if (!allocatable)
        throw std::bad_alloc();

    return ::operator new(size);
}

void operator delete[](void* ptr) throw()
{
    ::operator delete(ptr);
}

struct my_exception{};

void copy_failed_test()
{
    hamigaki::exception_ptr p;
#if defined(BOOST_MSVC)
    try
    {
        throw my_exception();
    }
    catch (...)
    {
        allocatable = false;
        p = hamigaki::current_exception();
        allocatable = true;
    }
#else
    allocatable = false;
    p = hamigaki::copy_exception(my_exception());
    allocatable = true;
#endif

    try
    {
        hamigaki::rethrow_exception(p);
    }
    catch (const std::bad_alloc&)
    {
    }
    catch (const my_exception&)
    {
    }
    catch (...)
    {
        BOOST_ERROR("catch other");
    }
}

bool copyable = true;

struct other_exception{};

struct some_exception
{
    some_exception()
    {
    }

    some_exception(const some_exception&)
    {
        if (!copyable)
            throw other_exception();
    }
};

void copy_failed_test2()
{
    allocatable = true;

    hamigaki::exception_ptr p;
#if defined(BOOST_MSVC)
    try
    {
        throw some_exception();
    }
    catch (...)
    {
        copyable = false;
        p = hamigaki::current_exception();
        copyable = true;
    }
#else
    copyable = false;
    p = hamigaki::copy_exception(some_exception());
    copyable = true;
#endif

    try
    {
        hamigaki::rethrow_exception(p);
    }
    catch (const std::bad_alloc&)
    {
    }
    catch (const some_exception&)
    {
    }
    catch (...)
    {
        BOOST_ERROR("catch other");
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("exception copy failed test");
    test->add(BOOST_TEST_CASE(&copy_failed_test));
    test->add(BOOST_TEST_CASE(&copy_failed_test2));
    return test;
}
