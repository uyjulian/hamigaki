// registry.hpp: Win32 registry utility

// Copyright Takeshi Mouri 2007-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_WIN32_REGISTRY_HPP
#define HAMIGAKI_BJAM_UTIL_WIN32_REGISTRY_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/util/list.hpp>
#include <boost/optional.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam { namespace win32 {

HAMIGAKI_BJAM_DECL
string_list registry_values(
    const std::string& key, const boost::optional<std::string>& name);

HAMIGAKI_BJAM_DECL
string_list registry_subkey_names(const std::string& key);

HAMIGAKI_BJAM_DECL
string_list registry_value_names(const std::string& key);

} } } // End namespaces win32, bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_UTIL_WIN32_REGISTRY_HPP
