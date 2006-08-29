//  generator_test.cpp: test case for generator

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/coroutine for library home page.

#include <hamigaki/coroutine/generator.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/test/unit_test.hpp>

namespace coro = hamigaki::coroutine;
namespace ut = boost::unit_test;

typedef coro::generator<int> generator_type;

class count_generator_body
{
public:
    count_generator_body(int min, int max)
        : min_(min), max_(max)
    {
    }

    int operator()(generator_type::self& self)
    {
        for (int i = min_; i < max_-1; ++i)
            self.yield(i);
        return max_-1;
    }

private:
    int min_;
    int max_;
};

class count_generator_body2
{
public:
    count_generator_body2(int min, int max)
        : min_(min), max_(max)
    {
    }

    int operator()(generator_type::self& self)
    {
        for (int i = min_; i < max_; ++i)
            self.yield(i);
        self.exit();
    }

private:
    int min_;
    int max_;
};

void generator_test()
{
    {
        generator_type gen(count_generator_body(0, 10));
        generator_type end;

        BOOST_CHECK_EQUAL_COLLECTIONS(
            boost::make_counting_iterator(0),
            boost::make_counting_iterator(10),
            gen, end
        );
    }

    {
        generator_type gen(count_generator_body2(0, 10));
        generator_type end;

        BOOST_CHECK_EQUAL_COLLECTIONS(
            boost::make_counting_iterator(0),
            boost::make_counting_iterator(10),
            gen, end
        );
    }
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("generator test");
    test->add(BOOST_TEST_CASE(&generator_test));
    return test;
}
