// target_path_test.cpp: test cases for bjam_target_path

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "../impl/bjam_target_path.hpp"
#include <boost/test/minimal.hpp>

int test_main(int, char*[])
{
    std::map<std::string,std::string> props;
    props["OS"] = "NT";
    props["toolset"] = "msvc-8.0";
    props["variant"] = "debug";
    props["threading"] = "single";
    props["link"] = "shared";
    props["runtime-link"] = "shared";

    std::vector<std::string> reqs;
    reqs.push_back("<toolset>msvc-8.0:<threading>multi");
    reqs.push_back("<user-interface>gui");

    ::merge_requirements(props, reqs);

    std::string ph = ::bjam_target_path(props).string();
    BOOST_CHECK(ph == "msvc-8.0/debug/threading-multi/user-interface-gui");

    return 0;
}
