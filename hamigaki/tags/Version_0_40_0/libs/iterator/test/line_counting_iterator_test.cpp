// line_counting_iterator_test.cpp: test case for line_counting_iterator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#include <hamigaki/iterator/line_counting_iterator.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/test/unit_test.hpp>
#include <iterator>
#include <list>

namespace ut = boost::unit_test;

struct fwd_iter
    : boost::iterator_adaptor<
        fwd_iter
      , const char*
      , boost::use_default
      , std::forward_iterator_tag
    >
{
};

void category_test()
{
    typedef hamigaki::line_counting_iterator<const char*> iter_type1;

    BOOST_MPL_ASSERT_NOT((
        boost::is_convertible<
            boost::iterator_category<iter_type1>::type,
            std::random_access_iterator_tag
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_convertible<
            boost::iterator_category<iter_type1>::type,
            std::bidirectional_iterator_tag
        >
    ));


    typedef hamigaki::line_counting_iterator<
        std::list<int>::iterator
    > iter_type2;

    BOOST_MPL_ASSERT_NOT((
        boost::is_convertible<
            boost::iterator_category<iter_type2>::type,
            std::random_access_iterator_tag
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_convertible<
            boost::iterator_category<iter_type2>::type,
            std::bidirectional_iterator_tag
        >
    ));


    typedef hamigaki::line_counting_iterator<fwd_iter> iter_type3;

    BOOST_MPL_ASSERT_NOT((
        boost::is_convertible<
            boost::iterator_category<iter_type3>::type,
            std::bidirectional_iterator_tag
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_convertible<
            boost::iterator_category<iter_type3>::type,
            std::forward_iterator_tag
        >
    ));


    typedef hamigaki::line_counting_iterator<
        std::istreambuf_iterator<char>
    > iter_type4;

    BOOST_MPL_ASSERT_NOT((
        boost::is_convertible<
            boost::iterator_category<iter_type4>::type,
            std::forward_iterator_tag
        >
    ));

    BOOST_MPL_ASSERT((
        boost::is_convertible<
            boost::iterator_category<iter_type4>::type,
            std::input_iterator_tag
        >
    ));
}

void line_counting_iterator_test()
{
    const char src[] = "a\nb\n";

    hamigaki::line_counting_iterator<const char*> i(src, 1);
    hamigaki::line_counting_iterator<const char*> end(src+(sizeof(src)-1));

    BOOST_CHECK_EQUAL(i.line(), 1);
    BOOST_REQUIRE(i != end);
    BOOST_CHECK_EQUAL(*i, 'a');

    ++i;

    BOOST_CHECK_EQUAL(i.line(), 1);
    BOOST_REQUIRE(i != end);
    BOOST_CHECK_EQUAL(*i, '\n');

    ++i;

    BOOST_CHECK_EQUAL(i.line(), 2);
    BOOST_REQUIRE(i != end);
    BOOST_CHECK_EQUAL(*i, 'b');

    ++i;

    BOOST_CHECK_EQUAL(i.line(), 2);
    BOOST_REQUIRE(i != end);
    BOOST_CHECK_EQUAL(*i, '\n');

    ++i;

    BOOST_CHECK_EQUAL(i.line(), 3);
    BOOST_REQUIRE(i == end);

    --i;

    BOOST_CHECK_EQUAL(i.line(), 2);
    BOOST_REQUIRE(i != end);
    BOOST_CHECK_EQUAL(*i, '\n');
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("line_counting_iterator test");
    test->add(BOOST_TEST_CASE(&category_test));
    test->add(BOOST_TEST_CASE(&line_counting_iterator_test));
    return test;
}
