// bjam_grammar_id.hpp: IDs for bjam grammar

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_ID_HPP
#define HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_ID_HPP

#include <hamigaki/bjam2/bjam_config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

enum bjam_grammar_id
{
    run_id = 1,
    block_id,
    rules_id,
    local_set_stmt_id,
    assign_list_id,
    arglist_id,
    rule_id,
    block_stmt_id,
    include_stmt_id,
    invoke_stmt_id,
    set_stmt_id,
    set_on_stmt_id,
    assign_id,
    return_stmt_id,
    for_stmt_id,
    switch_stmt_id,
    cases_id,
    module_stmt_id,
    class_stmt_id,
    while_stmt_id,
    if_stmt_id,
    rule_stmt_id,
    on_stmt_id,
    actions_stmt_id,
    expr_id,
    and_expr_id,
    eq_expr_id,
    rel_expr_id,
    not_expr_id,
    prim_expr_id,
    lol_id,
    list_id,
    non_punct_id,
    arg_id,
    func_id,
    eflags_id,
    bindlist_id,
};

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_ID_HPP
