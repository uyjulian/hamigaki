// remove_all_test.cpp: test case for remove_all()

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include <hamigaki/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>

namespace fs_ex = hamigaki::filesystem;
namespace fs = boost::filesystem;
namespace ut = boost::unit_test;

void remove_all_test0()
{
    BOOST_CHECK_EQUAL(fs_ex::remove_all(fs::path("not_found")), 0ul);
}

void remove_all_test1()
{
    fs::path dir1("dir1");

    fs::create_directory(dir1);

    BOOST_CHECK_EQUAL(fs_ex::remove_all(dir1), 1ul);

    BOOST_CHECK(!fs_ex::exists(dir1));

    fs::remove(dir1);
}

void remove_all_test2()
{
    fs::path dir1("dir1");
    fs::path file(dir1 / "file");

    fs::create_directory(dir1);
    fs::ofstream f(file);
    f.close();

    BOOST_CHECK_EQUAL(fs_ex::remove_all(dir1), 2ul);

    BOOST_CHECK(!fs_ex::exists(dir1));
    BOOST_CHECK(!fs_ex::exists(file));

    fs::remove(file);
    fs::remove(dir1);
}

void remove_symlink_test()
{
    fs::path dir1("dir1");
    fs::path file(dir1 / "file");
    fs::path link("link_dir");

    fs::create_directory(dir1);
    fs::ofstream f(file);
    f.close();

    fs_ex::create_directory_symlink(dir1, link);

    BOOST_CHECK_EQUAL(fs_ex::remove_all(link), 1ul);

    BOOST_CHECK(!fs_ex::exists(link));
    BOOST_CHECK(fs_ex::exists(dir1));
    BOOST_CHECK(fs_ex::exists(file));

    fs::remove(link);
    fs::remove(file);
    fs::remove(dir1);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("remove all test");
    test->add(BOOST_TEST_CASE(&remove_all_test0));
    test->add(BOOST_TEST_CASE(&remove_all_test1));
    test->add(BOOST_TEST_CASE(&remove_all_test2));
    test->add(BOOST_TEST_CASE(&remove_symlink_test));
    return test;
}
