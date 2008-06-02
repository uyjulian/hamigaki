// glob_test.cpp: test case for glob.hpp

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam2/util/glob.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam2;
namespace fs = boost::filesystem;
namespace ut = boost::unit_test;

std::string hamigaki_root;

void glob_test()
{
    bjam::string_list result;
    std::string expect;

    bjam::file_status_cache cache;
    cache.working_directory(fs::current_path<fs::path>().directory_string());

#if defined(BOOST_WINDOWS)
    expect = hamigaki_root + "\\Jamfile.v2";
#else
    expect = hamigaki_root + "/Jamfile.v2";
#endif

    result = bjam::glob(cache, hamigaki_root, "J*.v2");
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());


    result = bjam::glob(cache, hamigaki_root, "j*.v2", true);
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());


#if defined(BOOST_WINDOWS)
    expect = "./.\\Jamfile.v2";
#else
    expect = "././Jamfile.v2";
#endif
    result = bjam::glob(cache, "./.", "J*.v2");
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());
}

void glob_recursive_test()
{
    bjam::string_list result;
    std::string pattern;
    std::string expect;

    bjam::file_status_cache cache;
    cache.working_directory(fs::current_path<fs::path>().directory_string());

    pattern = hamigaki_root + "/libs/*/build/J*.v2";
#if defined(BOOST_WINDOWS)
    expect = hamigaki_root + "\\libs\\bjam\\build\\Jamfile.v2";
#else
    expect = hamigaki_root + "/libs/bjam/build/Jamfile.v2";
#endif
    result = bjam::glob_recursive(cache, pattern);
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());


    pattern = "././J*.v2";
#if defined(BOOST_WINDOWS)
    expect = ".\\.\\Jamfile.v2";
#else
    expect = "././Jamfile.v2";
#endif
    result = bjam::glob_recursive(cache, pattern);
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());
}

ut::test_suite* init_unit_test_suite(int argc, char* argv[])
{
    if (argc != 2)
        return 0;

    hamigaki_root = argv[1];

    ut::test_suite* test = BOOST_TEST_SUITE("glob test");
    test->add(BOOST_TEST_CASE(&glob_test));
    test->add(BOOST_TEST_CASE(&glob_recursive_test));
    return test;
}
