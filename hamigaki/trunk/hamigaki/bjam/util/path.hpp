// path.hpp: path utilities

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_PATH_HPP
#define HAMIGAKI_BJAM_UTIL_PATH_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/util/list.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

struct path_components
{
    std::string grist;
    std::string root;
    std::string dir;
    std::string base;
    std::string suffix;
    std::string member;
};

HAMIGAKI_BJAM_DECL void split_path(path_components& ph, const std::string& s);
HAMIGAKI_BJAM_DECL std::string make_path(const path_components& ph);

HAMIGAKI_BJAM_DECL std::string tmp_directory();
HAMIGAKI_BJAM_DECL std::string tmp_filename();
HAMIGAKI_BJAM_DECL std::string tmp_file_path();

HAMIGAKI_BJAM_DECL std::string normalize_path(const string_list& parts);

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_UTIL_PATH_HPP
