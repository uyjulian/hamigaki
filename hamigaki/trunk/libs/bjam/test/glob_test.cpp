// glob_test.cpp: test case for glob.hpp

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/util/glob.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace fs = boost::filesystem;
namespace ut = boost::unit_test;

std::string hamigaki_root;

void glob_test()
{
    bjam::string_list result;
    std::string expect;

    fs::path work = fs::current_path<fs::path>();

#if defined(BOOST_WINDOWS)
    expect = hamigaki_root + "\\Jamfile.v2";
#else
    expect = hamigaki_root + "/Jamfile.v2";
#endif

    result = bjam::glob(work.directory_string(), hamigaki_root, "J*.v2");
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());


    result = bjam::glob(
        work.directory_string(), hamigaki_root, "j*.v2", true);
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());


#if defined(BOOST_WINDOWS)
    expect = "./.\\Jamfile.v2";
#else
    expect = "././Jamfile.v2";
#endif
    result = bjam::glob(work.directory_string(), "./.", "J*.v2");
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());
}

void glob_recursive_test()
{
    bjam::string_list result;
    std::string pattern;
    std::string expect;

    fs::path work = fs::current_path<fs::path>();

    pattern = hamigaki_root + "/libs/*/build/J*.v2";
#if defined(BOOST_WINDOWS)
    expect = hamigaki_root + "\\libs\\bjam\\build\\Jamfile.v2";
#else
    expect = hamigaki_root + "/libs/bjam/build/Jamfile.v2";
#endif
    result = bjam::glob_recursive(work.directory_string(), pattern);
    BOOST_CHECK(std::find(result.begin(),result.end(),expect) != result.end());


    pattern = "././J*.v2";
#if defined(BOOST_WINDOWS)
    expect = ".\\.\\Jamfile.v2";
#else
    expect = "././Jamfile.v2";
#endif
    result = bjam::glob_recursive(work.directory_string(), pattern);
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
