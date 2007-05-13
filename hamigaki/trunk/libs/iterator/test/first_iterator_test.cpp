// first_iterator_test.cpp: test case for first_iterator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#include <hamigaki/iterator/first_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_convertible.hpp>

namespace ut = boost::unit_test;

template<class Pointer, class Category>
class test_iterator
    : public boost::iterator_adaptor<
        test_iterator<Pointer,Category>,
        Pointer,
        boost::use_default,
        Category
    >
{
    friend class boost::iterator_core_access;

    typedef boost::iterator_adaptor<
        test_iterator<Pointer,Category>,
        Pointer,
        boost::use_default,
        Category
    > base_type;

public:
    test_iterator() {}

    explicit test_iterator(Pointer x) : base_type(x)
    {
    }
};

template<class Category>
void first_iterator_test_mutable()
{
    typedef test_iterator<
        std::pair<int,int>*,
        Category
    > test_iter;

    typedef hamigaki::first_iterator<test_iter> iter_type;

#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    BOOST_MPL_ASSERT((boost::is_convertible<
        typename std::iterator_traits<iter_type>::iterator_category,
        Category
    >));
#endif

    std::pair<int,int> data[10];
    for (int i = 0; i < 10; ++i)
    {
        data[i].first = i*2;
        data[i].second = i*2+1;
    }

    iter_type iter((test_iter(data)));
    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK_EQUAL(*iter, i*2);
        *iter = -*iter;
        ++iter;
    }

    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK_EQUAL(data[i].first, -i*2);
        BOOST_CHECK_EQUAL(data[i].second, i*2+1);
    }
}

template<class Category>
void first_iterator_test_const()
{
    typedef test_iterator<
        const std::pair<int,int>*,
        Category
    > test_iter;

    typedef hamigaki::first_iterator<test_iter> iter_type;

#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    BOOST_MPL_ASSERT((boost::is_convertible<
        typename std::iterator_traits<iter_type>::iterator_category,
        Category
    >));
#endif

    std::pair<int,int> data[10];
    for (int i = 0; i < 10; ++i)
    {
        data[i].first = i*2;
        data[i].second = i*2+1;
    }

    iter_type iter((test_iter(data)));
    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK_EQUAL(*iter, i*2);
        ++iter;
    }
}

template<class Pointer, class Category>
class non_ref_iterator
    : public boost::iterator_adaptor<
        non_ref_iterator<Pointer,Category>,
        Pointer,
        boost::use_default,
        Category,
        typename std::iterator_traits<Pointer>::value_type
    >
{
    typedef boost::iterator_adaptor<
        non_ref_iterator<Pointer,Category>,
        Pointer,
        boost::use_default,
        Category,
        typename std::iterator_traits<Pointer>::value_type
    > base_type;

public:
    non_ref_iterator() {}

    explicit non_ref_iterator(Pointer x) : base_type(x)
    {
    }
};

template<class Category>
void first_iterator_test_non_ref()
{
    typedef non_ref_iterator<
        const std::pair<int,int>*,
        Category
    > test_iter;

    typedef hamigaki::first_iterator<test_iter> iter_type;

#if !BOOST_WORKAROUND(__BORLANDC__, BOOST_TESTED_AT(0x582))
    BOOST_MPL_ASSERT((boost::is_convertible<
        typename std::iterator_traits<iter_type>::iterator_category,
        Category
    >));
#endif

    std::pair<int,int> data[10];
    for (int i = 0; i < 10; ++i)
    {
        data[i].first = i*2;
        data[i].second = i*2+1;
    }

    iter_type iter((test_iter(data)));
    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK_EQUAL(*iter, i*2);
        ++iter;
    }
}

void first_iterator_test()
{
    first_iterator_test_mutable<std::input_iterator_tag>();
    first_iterator_test_mutable<std::forward_iterator_tag>();
    first_iterator_test_mutable<std::bidirectional_iterator_tag>();
    first_iterator_test_mutable<std::random_access_iterator_tag>();

    first_iterator_test_const<std::input_iterator_tag>();
    first_iterator_test_const<std::forward_iterator_tag>();
    first_iterator_test_const<std::bidirectional_iterator_tag>();
    first_iterator_test_const<std::random_access_iterator_tag>();

    first_iterator_test_non_ref<std::input_iterator_tag>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("first_iterator test");
    test->add(BOOST_TEST_CASE(&first_iterator_test));
    return test;
}
