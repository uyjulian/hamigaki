//  second_iterator_test.cpp: test case for second_iterator

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hamigaki/iterator/second_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/static_assert.hpp>

namespace ut = boost::unit_test;

template<class Pointer, class CategoryOrTraversal>
class test_iterator
    : public boost::iterator_adaptor<
        test_iterator<Pointer,CategoryOrTraversal>,
        Pointer,
        boost::use_default,
        CategoryOrTraversal
    >
{
    friend class boost::iterator_core_access;

public:
    test_iterator() {}

    explicit test_iterator(Pointer x)
        : test_iterator::iterator_adaptor_(x)
    {
    }
};

template<class CategoryOrTraversal, class Category>
void second_iterator_test_mutable()
{
    typedef test_iterator<
        std::pair<int,int>*,
        CategoryOrTraversal
    > test_iter;

    typedef hamigaki::second_iterator<test_iter> iter_type;

    typedef typename boost::is_convertible<
        typename std::iterator_traits<iter_type>::iterator_category,
        Category
    >::type category_check;

    BOOST_STATIC_ASSERT(category_check::value);

    std::pair<int,int> data[10];
    for (int i = 0; i < 10; ++i)
    {
        data[i].first = i*2;
        data[i].second = i*2+1;
    }

    iter_type iter((test_iter(data)));
    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK_EQUAL(*iter, i*2+1);
        *iter = -*iter;
        ++iter;
    }

    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK_EQUAL(data[i].first,  i*2);
        BOOST_CHECK_EQUAL(data[i].second, -(i*2+1));
    }
}

template<class CategoryOrTraversal, class Category>
void second_iterator_test_const()
{
    typedef test_iterator<
        const std::pair<int,int>*,
        CategoryOrTraversal
    > test_iter;

    typedef hamigaki::second_iterator<test_iter> iter_type;

    typedef typename boost::is_convertible<
        typename std::iterator_traits<iter_type>::iterator_category,
        Category
    >::type category_check;

    BOOST_STATIC_ASSERT(category_check::value);

    std::pair<int,int> data[10];
    for (int i = 0; i < 10; ++i)
    {
        data[i].first = i*2;
        data[i].second = i*2+1;
    }

    iter_type iter((test_iter(data)));
    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK_EQUAL(*iter, i*2+1);
        ++iter;
    }
}

void second_iterator_test()
{
    second_iterator_test_mutable<
        boost::single_pass_traversal_tag, std::input_iterator_tag>();
    second_iterator_test_mutable<
        boost::forward_traversal_tag, std::forward_iterator_tag>();
    second_iterator_test_mutable<
        boost::bidirectional_traversal_tag, std::bidirectional_iterator_tag>();
    second_iterator_test_mutable<
        boost::random_access_traversal_tag, std::random_access_iterator_tag>();

    second_iterator_test_mutable<
        std::input_iterator_tag, std::input_iterator_tag>();
    second_iterator_test_mutable<
        std::forward_iterator_tag, std::forward_iterator_tag>();
    second_iterator_test_mutable<
        std::bidirectional_iterator_tag, std::bidirectional_iterator_tag>();
    second_iterator_test_mutable<
        std::random_access_iterator_tag, std::random_access_iterator_tag>();

    second_iterator_test_const<
        boost::single_pass_traversal_tag, std::input_iterator_tag>();
    second_iterator_test_const<
        boost::forward_traversal_tag, std::forward_iterator_tag>();
    second_iterator_test_const<
        boost::bidirectional_traversal_tag, std::bidirectional_iterator_tag>();
    second_iterator_test_const<
        boost::random_access_traversal_tag, std::random_access_iterator_tag>();

    second_iterator_test_const<
        std::input_iterator_tag, std::input_iterator_tag>();
    second_iterator_test_const<
        std::forward_iterator_tag, std::forward_iterator_tag>();
    second_iterator_test_const<
        std::bidirectional_iterator_tag, std::bidirectional_iterator_tag>();
    second_iterator_test_const<
        std::random_access_iterator_tag, std::random_access_iterator_tag>();
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("second_iterator test");
    test->add(BOOST_TEST_CASE(&second_iterator_test));
    return test;
}
