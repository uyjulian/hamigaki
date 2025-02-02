// bjam_grammar.hpp: bjam grammar

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_HPP
#define HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <hamigaki/bjam2/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam2/grammars/bjam_grammar_id.hpp>
#include <hamigaki/bjam2/util/argument_parser.hpp>
#include <hamigaki/bjam2/util/keyword_parser.hpp>
#include <hamigaki/bjam2/util/parse_tree.hpp>
#include <hamigaki/bjam2/util/skip_parser.hpp>
#include <hamigaki/bjam2/util/string_parser.hpp>
#include <boost/spirit/tree/parse_tree.hpp>
#include <boost/spirit/core.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam2 {

struct bjam_grammar : boost::spirit::grammar<bjam_grammar>
{
    template<class ScannerT>
    struct definition
    {
        typedef boost::spirit::rule<
            ScannerT, boost::spirit::dynamic_parser_tag
        > rule_t;

        rule_t run;
        rule_t block;
        rule_t rules;
        rule_t assign_list;
        rule_t arglist;
        rule_t rule;
        rule_t local_set_stmt;
        rule_t block_stmt;
        rule_t include_stmt;
        rule_t invoke_stmt;
        rule_t set_stmt;
        rule_t set_on_stmt;
        rule_t assign;
        rule_t return_stmt;
        rule_t for_stmt;
        rule_t switch_stmt;
        rule_t cases;
        rule_t module_stmt;
        rule_t class_stmt;
        rule_t while_stmt;
        rule_t if_stmt;
        rule_t rule_stmt;
        rule_t on_stmt;
        rule_t actions_stmt;
        rule_t expr;
        rule_t and_expr;
        rule_t eq_expr;
        rule_t rel_expr;
        rule_t not_expr;
        rule_t prim_expr;
        rule_t lol;
        rule_t list;
        rule_t non_punct;
        rule_t arg;
        rule_t func;
        rule_t eflags;
        rule_t bindlist;

        definition(const bjam_grammar&)
        {
            using namespace boost::spirit;

            run
                =   !rules
                    >> end_p
                ;

            block
                =   !rules
                ;

            rules
                =   rule >> !rules
                |   local_set_stmt
                ;

            local_set_stmt
                =   no_node_d[keyword_p("local")]
                    >> list
                    >> !assign_list
                    >> no_node_d[keyword_p(";")]
                    >> block
                ;

            assign_list
                =   no_node_d[keyword_p("=")] >> list
                ;

            arglist
                =   no_node_d[keyword_p("(")]
                    >> lol
                    >> no_node_d[keyword_p(")")]
                ;

            rule
                =   block_stmt
                |   include_stmt
                |   invoke_stmt
                |   set_stmt
                |   set_on_stmt
                |   return_stmt
                |   for_stmt
                |   switch_stmt
                |   module_stmt
                |   class_stmt
                |   while_stmt
                |   if_stmt
                |   rule_stmt
                |   on_stmt
                |   actions_stmt
                ;

            block_stmt
                =   no_node_d[keyword_p("{")]
                    >> block
                    >> no_node_d[keyword_p("}")]
                ;

            include_stmt
                =   no_node_d[keyword_p("include")]
                    >> list
                    >> no_node_d[keyword_p(";")]
                ;

            invoke_stmt
                =   arg_p
                    >> lol
                    >> no_node_d[keyword_p(";")]
                ;

            set_stmt
                =   arg
                    >> assign
                    >> list
                    >> no_node_d[keyword_p(";")]
                ;

            set_on_stmt
                =   arg
                    >> no_node_d[keyword_p("on")]
                    >> list
                    >> assign
                    >> list
                    >> no_node_d[keyword_p(";")]
                ;

            assign
                =   keyword_p("=")
                |   keyword_p("+=")
                |   keyword_p("?=")
                |   keyword_p("default")
                    >> keyword_p("=")
                ;

            return_stmt
                =   no_node_d[keyword_p("return")]
                    >> list
                    >> no_node_d[keyword_p(";")]
                ;

            for_stmt
                =   no_node_d[keyword_p("for")]
                    >> !keyword_p("local")
                    >> arg_p
                    >> no_node_d[keyword_p("in")]
                    >> list
                    >> no_node_d[keyword_p("{")]
                    >> block
                    >> no_node_d[keyword_p("}")]
                ;

            switch_stmt
                =   no_node_d[keyword_p("switch")]
                    >> list
                    >> no_node_d[keyword_p("{")]
                    >> cases
                    >> no_node_d[keyword_p("}")]
                ;

            cases
                =  *(   no_node_d[keyword_p("case")]
                        >> arg_p
                        >> no_node_d[keyword_p(":")]
                        >> block
                    )
                ;

            module_stmt
                =   no_node_d[keyword_p("module")]
                    >> list
                    >> no_node_d[keyword_p("{")]
                    >> block
                    >> no_node_d[keyword_p("}")]
                ;

            class_stmt
                =   no_node_d[keyword_p("class")]
                    >> lol
                    >> no_node_d[keyword_p("{")]
                    >> block
                    >> no_node_d[keyword_p("}")]
                ;

            while_stmt
                =   no_node_d[keyword_p("while")]
                    >> expr
                    >> no_node_d[keyword_p("{")]
                    >> block
                    >> no_node_d[keyword_p("}")]
                ;

            if_stmt
                =   no_node_d[keyword_p("if")]
                    >> expr
                    >> no_node_d[keyword_p("{")]
                    >> block
                    >> no_node_d[keyword_p("}")]
                    >> !( no_node_d[keyword_p("else")] >> rule )
                ;

            rule_stmt
                =   !keyword_p("local")
                    >> no_node_d[keyword_p("rule")]
                    >> arg_p
                    >> !arglist
                    >> rule
                ;

            on_stmt
                =   no_node_d[keyword_p("on")]
                    >> arg
                    >> rule
                ;

            actions_stmt
                =   no_node_d[keyword_p("actions")]
                    >> eflags
                    >> arg_p
                    >> !bindlist
                    >> eps_p(keyword_p("{"))
                    >> lexeme_d
                    [
                        no_node_d[ch_p('{')]
                        >> string_p
                    ]
                    >> no_node_d[keyword_p("}")]
                ;

            expr
                =   and_expr
                    %   no_node_d
                        [
                            (   keyword_p("|")
                            |   keyword_p("||")
                            )
                        ]
                ;

            and_expr
                =   eq_expr
                    %   no_node_d
                        [
                            (   keyword_p("&")
                            |   keyword_p("&&")
                            )
                        ]
                ;

            eq_expr
                =   rel_expr
                    %   (   keyword_p("=")
                        |   keyword_p("!=")
                        )
                ;

            rel_expr
                =   not_expr
                    %   (   keyword_p("<")
                        |   keyword_p("<=")
                        |   keyword_p(">")
                        |   keyword_p(">=")
                        )
                ;

            not_expr
                =   *keyword_p("!") >> prim_expr
                ;

            prim_expr
                =   arg >> !(no_node_d[keyword_p("in")]  >> list)
                |   no_node_d[keyword_p("(")]
                    >> expr
                    >> no_node_d[keyword_p(")")]
                ;

            lol
                =   list % keyword_p(":")
                ;

            list
                = *non_punct
                ;

            non_punct
                =   non_punct_p
                |   no_node_d[keyword_p("[")]
                    >> func
                    >> no_node_d[keyword_p("]")]
                ;

            arg
                =   arg_p
                |   no_node_d[keyword_p("[")]
                    >> func
                    >> no_node_d[keyword_p("]")]
                ;

            func
                =   arg >> lol
                |   keyword_p("on") >> arg >> arg >> lol
                |   keyword_p("on") >> arg
                    >> keyword_p("return") >> list
                ;

            eflags
                =  *(   keyword_p("updated")
                    |   keyword_p("together")
                    |   keyword_p("ignore")
                    |   keyword_p("quietly")
                    |   keyword_p("piecemeal")
                    |   keyword_p("existing")
                    )
                ;

            bindlist
                =   no_node_d[keyword_p("bind")] >> list
                ;

            run.set_id(run_id);
            block.set_id(block_id);
            rules.set_id(rules_id);
            local_set_stmt.set_id(local_set_stmt_id);
            assign_list.set_id(assign_list_id);
            arglist.set_id(arglist_id);
            rule.set_id(rule_id);
            block_stmt.set_id(block_stmt_id);
            include_stmt.set_id(include_stmt_id);
            invoke_stmt.set_id(invoke_stmt_id);
            set_stmt.set_id(set_stmt_id);
            set_on_stmt.set_id(set_on_stmt_id);
            assign.set_id(assign_id);
            return_stmt.set_id(return_stmt_id);
            for_stmt.set_id(for_stmt_id);
            switch_stmt.set_id(switch_stmt_id);
            cases.set_id(cases_id);
            module_stmt.set_id(module_stmt_id);
            class_stmt.set_id(class_stmt_id);
            while_stmt.set_id(while_stmt_id);
            if_stmt.set_id(if_stmt_id);
            rule_stmt.set_id(rule_stmt_id);
            on_stmt.set_id(on_stmt_id);
            actions_stmt.set_id(actions_stmt_id);
            expr.set_id(expr_id);
            and_expr.set_id(and_expr_id);
            eq_expr.set_id(eq_expr_id);
            rel_expr.set_id(rel_expr_id);
            not_expr.set_id(not_expr_id);
            prim_expr.set_id(prim_expr_id);
            lol.set_id(lol_id);
            list.set_id(list_id);
            non_punct.set_id(non_punct_id);
            arg.set_id(arg_id);
            func.set_id(func_id);
            eflags.set_id(eflags_id);
            bindlist.set_id(bindlist_id);

            BOOST_SPIRIT_DEBUG_RULE(run);
            BOOST_SPIRIT_DEBUG_RULE(block);
            BOOST_SPIRIT_DEBUG_RULE(rules);
            BOOST_SPIRIT_DEBUG_RULE(local_set_stmt);
            BOOST_SPIRIT_DEBUG_RULE(assign_list);
            BOOST_SPIRIT_DEBUG_RULE(arglist);
            BOOST_SPIRIT_DEBUG_RULE(rule);
            BOOST_SPIRIT_DEBUG_RULE(block_stmt);
            BOOST_SPIRIT_DEBUG_RULE(include_stmt);
            BOOST_SPIRIT_DEBUG_RULE(invoke_stmt);
            BOOST_SPIRIT_DEBUG_RULE(set_stmt);
            BOOST_SPIRIT_DEBUG_RULE(set_on_stmt);
            BOOST_SPIRIT_DEBUG_RULE(assign);
            BOOST_SPIRIT_DEBUG_RULE(return_stmt);
            BOOST_SPIRIT_DEBUG_RULE(for_stmt);
            BOOST_SPIRIT_DEBUG_RULE(switch_stmt);
            BOOST_SPIRIT_DEBUG_RULE(cases);
            BOOST_SPIRIT_DEBUG_RULE(module_stmt);
            BOOST_SPIRIT_DEBUG_RULE(while_stmt);
            BOOST_SPIRIT_DEBUG_RULE(if_stmt);
            BOOST_SPIRIT_DEBUG_RULE(rule_stmt);
            BOOST_SPIRIT_DEBUG_RULE(on_stmt);
            BOOST_SPIRIT_DEBUG_RULE(expr);
            BOOST_SPIRIT_DEBUG_RULE(and_expr);
            BOOST_SPIRIT_DEBUG_RULE(eq_expr);
            BOOST_SPIRIT_DEBUG_RULE(rel_expr);
            BOOST_SPIRIT_DEBUG_RULE(not_expr);
            BOOST_SPIRIT_DEBUG_RULE(prim_expr);
            BOOST_SPIRIT_DEBUG_RULE(lol);
            BOOST_SPIRIT_DEBUG_RULE(list);
            BOOST_SPIRIT_DEBUG_RULE(non_punct);
            BOOST_SPIRIT_DEBUG_RULE(arg);
            BOOST_SPIRIT_DEBUG_RULE(func);
            BOOST_SPIRIT_DEBUG_RULE(eflags);
            BOOST_SPIRIT_DEBUG_RULE(bindlist);
        }

        const rule_t& start() const { return run; }
    };
};

// FIXME: to avoid C2491
#if defined(HAMIGAKI_BJAM2_SOURCE)

#if HAMIGAKI_BJAM2_SEPARATE_GRAMMAR_INSTANTIATION != 0
    #define HAMIGAKI_BJAM2_GRAMMAR_GEN_INLINE
#else
    #define HAMIGAKI_BJAM2_GRAMMAR_GEN_INLINE inline
#endif 

template<class IteratorT>
HAMIGAKI_BJAM2_GRAMMAR_GEN_INLINE
hamigaki::bjam2::tree_parse_info<IteratorT>
bjam_grammar_gen<IteratorT>::parse_bjam_grammar(
    const IteratorT& first0, const IteratorT& last0)
{
    typedef hamigaki::line_counting_iterator<IteratorT> iter_t;
    typedef hamigaki::bjam2::node_val_data_factory<> node_factory_t;

    iter_t first(first0, 1);
    iter_t last(last0);

    bjam2::bjam_grammar g;
    bjam2::skip_parser skip;

    boost::spirit::tree_parse_info<iter_t,node_factory_t> tmp =
        bjam2::tree_parse(first, last, g, skip);

    hamigaki::bjam2::tree_parse_info<IteratorT> info;
    info.stop = tmp.stop.base();
    info.match = tmp.match;
    info.full = tmp.full;
    info.length = tmp.length;
    info.line = tmp.stop.line();
    info.trees.swap(tmp.trees);

    return info;
}

#undef HAMIGAKI_BJAM2_GRAMMAR_GEN_INLINE

#endif // !defined(HAMIGAKI_BJAM2_SOURCE)

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_GRAMMARS_BJAM_GRAMMAR_HPP
