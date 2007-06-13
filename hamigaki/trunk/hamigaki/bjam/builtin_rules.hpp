// builtin_rules.hpp: bjam builtin rules

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_BUILTIN_RULES_HPP
#define HAMIGAKI_BJAM_BUILTIN_RULES_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/util/list.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

class context;

namespace builtins
{

HAMIGAKI_BJAM_DECL string_list echo(context& ctx);
HAMIGAKI_BJAM_DECL string_list exit(context& ctx);
HAMIGAKI_BJAM_DECL string_list glob(context& ctx);
HAMIGAKI_BJAM_DECL string_list glob_recursive(context& ctx);

HAMIGAKI_BJAM_DECL string_list rulenames(context& ctx);
HAMIGAKI_BJAM_DECL string_list varnames(context& ctx);

HAMIGAKI_BJAM_DECL string_list import(context& ctx);
HAMIGAKI_BJAM_DECL string_list export_(context& ctx);

HAMIGAKI_BJAM_DECL string_list import_module(context& ctx);

HAMIGAKI_BJAM_DECL string_list instance(context& ctx);

} // namespace builtins

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_BUILTIN_RULES_HPP
