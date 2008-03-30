// file_time_test.cpp: test case for the file time functions

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

void file_time_test()
{
    const fs::path p("file.dat");

    fs::ofstream os(p);
    os.close();

    const fs_ex::timestamp& ts = fs_ex::timestamp::from_time_t(std::time(0));

    fs_ex::last_write_time(p, ts);
    fs_ex::last_access_time(p, ts);
    fs_ex::creation_time(p, ts);

    fs::remove(p);
}

void directory_time_test()
{
    const fs::path p("test_dir");

    fs::create_directory(p);

    const fs_ex::timestamp& ts = fs_ex::timestamp::from_time_t(std::time(0));

    fs_ex::last_write_time(p, ts);
    fs_ex::last_access_time(p, ts);
    fs_ex::creation_time(p, ts);

    fs::remove(p);
}

void symlink_time_test()
{
    const fs::path dp("test_dir");
    const fs::path p("test_symlink");

    fs::remove(p);
    fs::create_directory(dp);
    fs_ex::create_directory_symlink(dp, p);

    const fs_ex::timestamp& ts = fs_ex::timestamp::from_time_t(std::time(0));

    fs_ex::last_write_time(p, ts);
    fs_ex::last_access_time(p, ts);
    fs_ex::creation_time(p, ts);

    fs::remove(p);
    fs::remove(dp);
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("file time test");
    test->add(BOOST_TEST_CASE(&file_time_test));
    test->add(BOOST_TEST_CASE(&directory_time_test));
    test->add(BOOST_TEST_CASE(&symlink_time_test));
    return test;
}
