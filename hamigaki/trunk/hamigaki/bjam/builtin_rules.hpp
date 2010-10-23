// builtin_rules.hpp: bjam builtin rules

// Copyright Takeshi Mouri 2007-2010.
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

HAMIGAKI_BJAM_DECL string_list always(context& ctx);
HAMIGAKI_BJAM_DECL string_list depends(context& ctx);
HAMIGAKI_BJAM_DECL string_list echo(context& ctx);
HAMIGAKI_BJAM_DECL string_list exit(context& ctx);
HAMIGAKI_BJAM_DECL string_list glob(context& ctx);
HAMIGAKI_BJAM_DECL string_list glob_recursive(context& ctx);
HAMIGAKI_BJAM_DECL string_list includes(context& ctx);
HAMIGAKI_BJAM_DECL string_list rebuilds(context& ctx);
HAMIGAKI_BJAM_DECL string_list leaves(context& ctx);
HAMIGAKI_BJAM_DECL string_list match(context& ctx);
HAMIGAKI_BJAM_DECL string_list no_care(context& ctx);
HAMIGAKI_BJAM_DECL string_list not_file(context& ctx);
HAMIGAKI_BJAM_DECL string_list no_update(context& ctx);
HAMIGAKI_BJAM_DECL string_list temporary(context& ctx);
HAMIGAKI_BJAM_DECL string_list is_file(context& ctx);
HAMIGAKI_BJAM_DECL string_list hdr_macro(context& ctx);
HAMIGAKI_BJAM_DECL string_list fail_expected(context& ctx);
HAMIGAKI_BJAM_DECL string_list rm_old(context& ctx);
HAMIGAKI_BJAM_DECL string_list update(context& ctx);
HAMIGAKI_BJAM_DECL string_list update_now(context& ctx);
HAMIGAKI_BJAM_DECL string_list subst(context& ctx);
HAMIGAKI_BJAM_DECL string_list rule_names(context& ctx);
HAMIGAKI_BJAM_DECL string_list var_names(context& ctx);
HAMIGAKI_BJAM_DECL string_list delete_module(context& ctx);
HAMIGAKI_BJAM_DECL string_list import(context& ctx);
HAMIGAKI_BJAM_DECL string_list export_(context& ctx);
HAMIGAKI_BJAM_DECL string_list caller_module(context& ctx);
HAMIGAKI_BJAM_DECL string_list back_trace(context& ctx);
HAMIGAKI_BJAM_DECL string_list pwd(context& ctx);
HAMIGAKI_BJAM_DECL string_list search_for_target(context& ctx);
HAMIGAKI_BJAM_DECL string_list import_module(context& ctx);
HAMIGAKI_BJAM_DECL string_list imported_modules(context& ctx);
HAMIGAKI_BJAM_DECL string_list instance(context& ctx);
HAMIGAKI_BJAM_DECL string_list sort(context& ctx);
HAMIGAKI_BJAM_DECL string_list normalize_path(context& ctx);
HAMIGAKI_BJAM_DECL string_list calc(context& ctx);
HAMIGAKI_BJAM_DECL string_list native_rule(context& ctx);
HAMIGAKI_BJAM_DECL string_list has_native_rule(context& ctx);
HAMIGAKI_BJAM_DECL string_list user_module(context& ctx);
HAMIGAKI_BJAM_DECL string_list nearest_user_location(context& ctx);
HAMIGAKI_BJAM_DECL string_list check_if_file(context& ctx);

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
HAMIGAKI_BJAM_DECL string_list w32_getreg(context& ctx);
HAMIGAKI_BJAM_DECL string_list w32_getregnames(context& ctx);
#endif

HAMIGAKI_BJAM_DECL string_list shell(context& ctx);
HAMIGAKI_BJAM_DECL string_list md5(context& ctx);
HAMIGAKI_BJAM_DECL string_list file_open(context& ctx);
HAMIGAKI_BJAM_DECL string_list pad(context& ctx);
HAMIGAKI_BJAM_DECL string_list precious(context& ctx);
HAMIGAKI_BJAM_DECL string_list self_path(context& ctx);

} // namespace builtins

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_BUILTIN_RULES_HPP
