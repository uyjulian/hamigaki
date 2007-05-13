// status_test.cpp: test case for status()

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/unit_test.hpp>

namespace fs_ex = hamigaki::filesystem;
namespace fs = boost::filesystem;
namespace ut = boost::unit_test;

void not_found_test()
{
    const fs::path p("not_found.dat");

    fs_ex::file_status s = fs_ex::status(p);

    BOOST_CHECK(status_known(s));
    BOOST_CHECK(!exists(s));
    BOOST_CHECK(!is_regular(s));
    BOOST_CHECK(!is_directory(s));
    BOOST_CHECK(!is_symlink(s));
    BOOST_CHECK(!is_other(s));

    s = fs_ex::symlink_status(p);

    BOOST_CHECK(status_known(s));
    BOOST_CHECK(!exists(s));
    BOOST_CHECK(!is_regular(s));
    BOOST_CHECK(!is_directory(s));
    BOOST_CHECK(!is_symlink(s));
    BOOST_CHECK(!is_other(s));
}

void regular_test()
{
    const fs::path p("abc.dat");

    fs::ofstream os(p);
    os << "abc";
    os.close();

    fs_ex::file_status s = fs_ex::status(p);

    BOOST_CHECK_EQUAL(s.file_size(), static_cast<boost::uintmax_t>(3));

    BOOST_CHECK(status_known(s));
    BOOST_CHECK(exists(s));
    BOOST_CHECK(is_regular(s));
    BOOST_CHECK(!is_directory(s));
    BOOST_CHECK(!is_symlink(s));
    BOOST_CHECK(!is_other(s));

    s = fs_ex::symlink_status(p);

    BOOST_CHECK_EQUAL(s.file_size(), static_cast<boost::uintmax_t>(3));

    BOOST_CHECK(status_known(s));
    BOOST_CHECK(exists(s));
    BOOST_CHECK(is_regular(s));
    BOOST_CHECK(!is_directory(s));
    BOOST_CHECK(!is_symlink(s));
    BOOST_CHECK(!is_other(s));

    fs::remove(p);
}

void directory_test()
{
    const fs::path p("test_dir");

    fs::create_directory(p);

    fs_ex::file_status s = fs_ex::status(p);

    BOOST_CHECK(status_known(s));
    BOOST_CHECK(exists(s));
    BOOST_CHECK(!is_regular(s));
    BOOST_CHECK(is_directory(s));
    BOOST_CHECK(!is_symlink(s));
    BOOST_CHECK(!is_other(s));

    s = fs_ex::symlink_status(p);

    BOOST_CHECK(status_known(s));
    BOOST_CHECK(exists(s));
    BOOST_CHECK(!is_regular(s));
    BOOST_CHECK(is_directory(s));
    BOOST_CHECK(!is_symlink(s));
    BOOST_CHECK(!is_other(s));

    fs::remove(p);
}

void symlink_test()
{
    const fs::path dp("test_dir");
    const fs::path p("test_symlink");

    fs::remove(p);
    fs::create_directory(dp);
    fs_ex::create_directory_symlink(dp, p);

    fs_ex::file_status s = fs_ex::status(p);

    BOOST_CHECK(status_known(s));
    BOOST_CHECK(exists(s));
    BOOST_CHECK(!is_regular(s));
    BOOST_CHECK(is_directory(s));
    BOOST_CHECK(!is_symlink(s));
    BOOST_CHECK(!is_other(s));

    s = fs_ex::symlink_status(p);

    BOOST_CHECK(status_known(s));
    BOOST_CHECK(exists(s));
    BOOST_CHECK(!is_regular(s));
    BOOST_CHECK(!is_directory(s));
    BOOST_CHECK(is_symlink(s));
    BOOST_CHECK(!is_other(s));

    fs::remove(p);
    fs::remove(dp);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("file status test");
    test->add(BOOST_TEST_CASE(&not_found_test));
    test->add(BOOST_TEST_CASE(&regular_test));
    test->add(BOOST_TEST_CASE(&directory_test));
    test->add(BOOST_TEST_CASE(&symlink_test));
    return test;
}
