// path_test.cpp: test case for path.hpp

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam/util/path.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/test/unit_test.hpp>

namespace bjam = hamigaki::bjam;
namespace ut = boost::unit_test;

void split_path_test()
{
    bjam::path_components ph;

    bjam::split_path(ph, "");
    BOOST_CHECK_EQUAL(ph.grist, "");
    BOOST_CHECK_EQUAL(ph.root, "");
    BOOST_CHECK_EQUAL(ph.dir, "");
    BOOST_CHECK_EQUAL(ph.base, "");
    BOOST_CHECK_EQUAL(ph.suffix, "");
    BOOST_CHECK_EQUAL(ph.member, "");

    bjam::split_path(ph, "<grist>/dir/base.suffix(member)");
    BOOST_CHECK_EQUAL(ph.grist, "<grist>");
    BOOST_CHECK_EQUAL(ph.root, "");
    BOOST_CHECK_EQUAL(ph.dir, "/dir/");
    BOOST_CHECK_EQUAL(ph.base, "base");
    BOOST_CHECK_EQUAL(ph.suffix, ".suffix");
    BOOST_CHECK_EQUAL(ph.member, "member");

#if defined(BOOST_WINDOWS)
    bjam::split_path(ph, "<grist>C:\\dir\\base.suffix(member)");
    BOOST_CHECK_EQUAL(ph.grist, "<grist>");
    BOOST_CHECK_EQUAL(ph.root, "");
    BOOST_CHECK_EQUAL(ph.dir, "C:\\dir\\");
    BOOST_CHECK_EQUAL(ph.base, "base");
    BOOST_CHECK_EQUAL(ph.suffix, ".suffix");
    BOOST_CHECK_EQUAL(ph.member, "member");
#endif
}

void make_path_test()
{
    bjam::path_components ph;

    ph.grist = "";
    ph.root = "";
    ph.dir = "";
    ph.base = "";
    ph.suffix = "";
    ph.member = "";
    BOOST_CHECK_EQUAL(bjam::make_path(ph), "");

    ph.grist = "<grist>";
    ph.root = "";
    ph.dir = "/dir/";
    ph.base = "base";
    ph.suffix = ".suffix";
    ph.member = "member";
    BOOST_CHECK_EQUAL(bjam::make_path(ph), "<grist>/dir/base.suffix(member)");

    ph.grist = "<grist>";
    ph.root = "";
    ph.dir = "/dir";
    ph.base = "base";
    ph.suffix = ".suffix";
    ph.member = "member";
#if defined(BOOST_WINDOWS)
    BOOST_CHECK_EQUAL(bjam::make_path(ph), "<grist>/dir\\base.suffix(member)");
#else
    BOOST_CHECK_EQUAL(bjam::make_path(ph), "<grist>/dir/base.suffix(member)");
#endif

    ph.grist = "grist";
    ph.root = "";
    ph.dir = "/dir/";
    ph.base = "base";
    ph.suffix = ".suffix";
    ph.member = "member";
    BOOST_CHECK_EQUAL(bjam::make_path(ph), "<grist>/dir/base.suffix(member)");

    ph.grist = "<grist>";
    ph.root = "/";
    ph.dir = "/dir/";
    ph.base = "base";
    ph.suffix = ".suffix";
    ph.member = "member";
    BOOST_CHECK_EQUAL(bjam::make_path(ph), "<grist>/dir/base.suffix(member)");

    ph.grist = "<grist>";
    ph.root = "/root/";
    ph.dir = "dir/";
    ph.base = "base";
    ph.suffix = ".suffix";
    ph.member = "member";
    BOOST_CHECK_EQUAL(
        bjam::make_path(ph), "<grist>/root/dir/base.suffix(member)");

    ph.grist = "<grist>";
    ph.root = "/root/";
    ph.dir = "dir/";
    ph.base = "";
    ph.suffix = "";
    ph.member = "member";
    BOOST_CHECK_EQUAL(bjam::make_path(ph), "<grist>/root/dir(member)");

#if defined(BOOST_WINDOWS)
    ph.grist = "<grist>";
    ph.root = "C:\\";
    ph.dir = "\\dir\\";
    ph.base = "base";
    ph.suffix = ".suffix";
    ph.member = "member";
    BOOST_CHECK_EQUAL(bjam::make_path(ph), "<grist>\\dir\\base.suffix(member)");

    ph.grist = "<grist>";
    ph.root = "C:\\";
    ph.dir = "dir\\";
    ph.base = "base";
    ph.suffix = ".suffix";
    ph.member = "member";
    BOOST_CHECK_EQUAL(
        bjam::make_path(ph), "<grist>C:\\dir\\base.suffix(member)");
#endif
}

void normalize_path_test()
{
    bjam::string_list parts;

    BOOST_CHECK_EQUAL(bjam::normalize_path(parts), std::string("."));

    parts = boost::assign::list_of("/");
    BOOST_CHECK_EQUAL(bjam::normalize_path(parts), std::string("/"));

    parts = boost::assign::list_of("/")("tmp");
    BOOST_CHECK_EQUAL(bjam::normalize_path(parts), std::string("/tmp"));

    parts = boost::assign::list_of("/")("tmp")("..")("usr")(".")("include");
    BOOST_CHECK_EQUAL(bjam::normalize_path(parts), std::string("/usr/include"));

    parts = boost::assign::list_of("usr")("..")("..")("src");
    BOOST_CHECK_EQUAL(bjam::normalize_path(parts), std::string("../src"));

    parts = boost::assign::list_of("..")("..")("src");
    BOOST_CHECK_EQUAL(bjam::normalize_path(parts), std::string("../../src"));

#if defined(BOOST_WINDOWS)
    parts = boost::assign::list_of("C:/tmp/dir")("..");
    BOOST_CHECK_EQUAL(bjam::normalize_path(parts), std::string("C:/tmp"));
#endif
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("path utilities test");
    test->add(BOOST_TEST_CASE(&split_path_test));
    test->add(BOOST_TEST_CASE(&make_path_test));
    test->add(BOOST_TEST_CASE(&normalize_path_test));
    return test;
}
