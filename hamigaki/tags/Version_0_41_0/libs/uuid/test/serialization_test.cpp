// serialization_test.cpp: a test for hamigaki::uuid serialization

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/uuid for library home page.

#include <hamigaki/uuid.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/test/unit_test.hpp>
#include <sstream>

namespace ut = boost::unit_test;

std::string save_to_string(const hamigaki::uuid& id)
{
    std::ostringstream os;
    boost::archive::text_oarchive oa(os);
    oa << id;
    return os.str();
}

hamigaki::uuid load_from_string(const std::string& s)
{
    std::istringstream is(s);
    boost::archive::text_iarchive ia(is);
    hamigaki::uuid id;
    ia >> id;
    return id;
}

void serialization_test()
{
    hamigaki::uuid u1("{F81D4FAE-7DEC-11D0-A765-00A0C91E6BF6}");
    hamigaki::uuid u2 = load_from_string(save_to_string(u1));
    BOOST_CHECK(u1 == u2);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("serialization test");
    test->add(BOOST_TEST_CASE(&serialization_test));
    return test;
}
