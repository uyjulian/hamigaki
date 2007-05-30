// bjam_grammar.hpp: bjam grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP

#include <hamigaki/bjam/grammars/bjam_actions.hpp>
#include <hamigaki/bjam/grammars/bjam_closures.hpp>
#include <hamigaki/bjam/util/argument_parser.hpp>
#include <hamigaki/bjam/util/keyword_parser.hpp>
#include <hamigaki/bjam/util/string_parser.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <hamigaki/spirit/phoenix/stl/clear.hpp>
#include <boost/spirit/dynamic/if.hpp>

namespace hamigaki { namespace bjam {

struct bjam_grammar : boost::spirit::grammar<bjam_grammar>
{
    bjam_grammar(bjam::context& ctx) : context(ctx)
    {
    }

    bjam::context& context;

    template<class ScannerT>
    struct definition
    {
        typedef boost::spirit::rule<ScannerT> rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename list_closure::context_t
        > list_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename lol_closure::context_t
        > lol_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename assign_closure::context_t
        > assign_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename set_stmt_closure::context_t
        > set_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename rule_stmt_closure::context_t
        > rule_stmt_rule_t;

        rule_t run;
        list_rule_t block, rules, assign_list;
        lol_rule_t arglist;
        list_rule_t rule;
        set_stmt_rule_t set_stmt;
        assign_rule_t assign;
        rule_stmt_rule_t rule_stmt;
        list_rule_t expr, and_expr, eq_expr, rel_expr, not_expr, prim_expr;
        rule_t cases, case_;
        lol_rule_t lol;
        list_rule_t list, non_punct, arg, func;
        rule_t eflags, eflag, bindlist;

        rule_t block_nocalc, rules_nocalc, assign_list_nocalc;
        rule_t arglist_nocalc, rule_nocalc;
        rule_t expr_nocalc, and_expr_nocalc, eq_expr_nocalc;
        rule_t rel_expr_nocalc, not_expr_nocalc, prim_expr_nocalc;
        rule_t cases_nocalc, case_nocalc;
        rule_t lol_nocalc, list_nocalc, non_punct_nocalc, arg_nocalc;
        rule_t func_nocalc, eflags_nocalc, eflag_nocalc, bindlist_nocalc;

        definition(const bjam_grammar& self)
        {
            namespace hp = hamigaki::phoenix;
            using namespace boost::spirit;
            using namespace ::phoenix;

            run
                =   !rules >> end_p
                ;

            block
                =   !rules
                ;

            rules
                =   rule >> !rules
                |   keyword_p("local")
                    >> list
                    >> !assign_list
                    >> keyword_p(";")
                    >> block
                ;

            assign_list
                =   keyword_p("=") >> list [assign_list.val = arg1]
                ;

            arglist
                =   keyword_p("(")
                    >> lol [arglist.val = arg1]
                    >> keyword_p(")")
                ;

            rule
                =   keyword_p("{") >> block >> keyword_p("}")
                |   keyword_p("include") >> list >> keyword_p(";")
                |   arg_p >> lol >> keyword_p(";")
                |   set_stmt
                |   arg >> keyword_p("on") >> list
                    >> assign >> list >> keyword_p(";")
                |   keyword_p("return")
                    >> list [rule.val = arg1]
                    >> keyword_p(";")
                |   keyword_p("for") >> !keyword_p("local") >> arg_p
                    >> keyword_p("in") >> list
                    >> keyword_p("{") >> block >> keyword_p("}")
                |   keyword_p("switch") >> list
                    >> keyword_p("{") >> cases >> keyword_p("}")
                |   keyword_p("module") >> list
                    >> keyword_p("{") >> block >> keyword_p("}") 
                |   keyword_p("class") >> lol
                    >> keyword_p("{") >> block >> keyword_p("}") 
                |   keyword_p("while") >> expr
                    >> keyword_p("{") >> block >> keyword_p("}") 
                |   keyword_p("if") >> expr
                    >> keyword_p("{") >> block >> keyword_p("}")
                    >> !( keyword_p("else") >> rule )
                |   rule_stmt
                |   keyword_p("on") >> arg >> rule
                |   keyword_p("actions") >> eflags >> arg_p >> !bindlist
                    >> eps_p(keyword_p("{"))
                    >> lexeme_d[ '{' >> string_p ]
                    >> keyword_p("}")
                ;

            set_stmt
                =   arg [set_stmt.names = arg1]
                    >> assign [set_stmt.mode = arg1]
                    >> list [set_stmt.val = arg1]
                    >> keyword_p(";")
                    [
                        var_set(
                            boost::ref(self.context), set_stmt.mode,
                            set_stmt.names, set_stmt.val
                        )
                    ]
                ;

            assign
                =   keyword_p("=") [assign.val = assign_mode::set]
                |   keyword_p("+=") [assign.val = assign_mode::append]
                |   keyword_p("?=") [assign.val = assign_mode::set_default]
                |   keyword_p("default")
                    >> keyword_p("=")
                    [
                        assign.val = assign_mode::set_default
                    ]
                ;

            rule_stmt
                =   eps_p [rule_stmt.exported = true]
                    >> !( keyword_p("local") [rule_stmt.exported = false] )
                    >> keyword_p("rule")
                    >> arg_p [rule_stmt.name = arg1]
                    >> !( arglist [rule_stmt.params = arg1] )
                    >> rule_nocalc
                    [
                        rule_set(
                            boost::ref(self.context),
                            rule_stmt.name, rule_stmt.params,
                            arg1, arg2, rule_stmt.exported
                        )
                    ]
                ;

            expr
                =   and_expr [expr.val = arg1]
                    >> *(
                            ( keyword_p("|") | keyword_p("||") )
                            >> if_p(expr.val)
                            [
                                and_expr_nocalc [hp::clear(expr.val)]
                            ]
                            .else_p
                            [
                                and_expr [expr.val = arg1]
                            ]
                        )
                ;

            and_expr
                =   eq_expr [and_expr.val = arg1]
                    >> *(
                            ( keyword_p("&") | keyword_p("&&") )
                            >> if_p(and_expr.val)
                            [
                                eq_expr
                                [
                                    if_(!arg1)
                                    [
                                        hp::clear(and_expr.val)
                                    ]
                                ]
                            ]
                            .else_p
                            [
                                eq_expr_nocalc [hp::clear(and_expr.val)]
                            ]
                        )
                ;

            eq_expr
                =   rel_expr [eq_expr.val = arg1]
                    >> *(   keyword_p("=")
                            >> rel_expr
                            [
                                if_(eq_expr.val == arg1)
                                [
                                    set_true(eq_expr.val, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(eq_expr.val)
                                ]
                            ]
                        |   keyword_p("!=")
                            >> rel_expr
                            [
                                if_(eq_expr.val != arg1)
                                [
                                    set_true(eq_expr.val, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(eq_expr.val)
                                ]
                            ]
                        )
                ;

            rel_expr
                =   not_expr [rel_expr.val = arg1]
                    >> *(   keyword_p("<")
                            >> not_expr
                            [
                                if_(rel_expr.val < arg1)
                                [
                                    set_true(rel_expr.val, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(rel_expr.val)
                                ]
                            ]
                        |   keyword_p("<=")
                            >> not_expr
                            [
                                if_(rel_expr.val <= arg1)
                                [
                                    set_true(rel_expr.val, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(rel_expr.val)
                                ]
                            ]
                        |   keyword_p(">")
                            >> not_expr
                            [
                                if_(rel_expr.val > arg1)
                                [
                                    set_true(rel_expr.val, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(rel_expr.val)
                                ]
                            ]
                        |   keyword_p(">=")
                            >> not_expr
                            [
                                if_(rel_expr.val >= arg1)
                                [
                                    set_true(rel_expr.val, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(rel_expr.val)
                                ]
                            ]
                        )
                ;

            not_expr
                =   keyword_p("!")
                    >> prim_expr
                    [
                        if_(arg1)
                        [
                            hp::clear(not_expr.val)
                        ]
                        .else_
                        [
                            set_true(not_expr.val, arg1)
                        ]
                    ]
                |   prim_expr [not_expr.val = arg1]
                ;

            prim_expr
                =   arg [prim_expr.val = arg1]
                    >> !(   keyword_p("in")
                            >> if_p(prim_expr.val)
                            [
                                list
                                [
                                    if_(includes(prim_expr.val, arg1))
                                    [
                                        set_true(prim_expr.val, arg1)
                                    ]
                                    .else_
                                    [
                                        hp::clear(prim_expr.val)
                                    ]
                                ]
                            ]
                            .else_p
                            [
                                list_nocalc
                                [
                                    set_true(prim_expr.val)
                                ]
                            ]
                        )
                |   keyword_p("(")
                    >> expr [prim_expr.val = arg1]
                    >> keyword_p(")")
                ;

            cases
                =   *case_
                ;

            case_
                =   keyword_p("case") >> arg_p >> keyword_p(":") >> block
                ;

            lol
                =   list [lol.val += arg1]
                    % keyword_p(":")
                ;

            list
                = *non_punct [list.val += arg1]
                ;

            non_punct
                =   non_punct_p
                    [
                        var_expand(
                            boost::ref(self.context), non_punct.val, arg1)
                    ]
                |   keyword_p("[")
                    >> func [non_punct.val = arg1]
                    >> keyword_p("]")
                ;

            arg
                =   arg_p
                    [
                        var_expand(boost::ref(self.context), arg.val, arg1)
                    ]
                |   keyword_p("[")
                    >> func [arg.val = arg1]
                    >> keyword_p("]")
                ;

            func
                =   arg >> lol
                |   keyword_p("on") >> arg >> arg >> lol
                |   keyword_p("on") >> arg >> keyword_p("return") >> list
                ;

            eflags
                = *eflag;

            eflag
                =   keyword_p("updated")
                |   keyword_p("together")
                |   keyword_p("ignore")
                |   keyword_p("quietly")
                |   keyword_p("piecemeal")
                |   keyword_p("existing")
                ;

            bindlist
                =   keyword_p("bind") >> list
                ;


            // "nocalc" versions

            block_nocalc
                =   !rules_nocalc
                ;

            rules_nocalc
                =   rule_nocalc >> !rules_nocalc
                |   keyword_p("local")
                    >> list_nocalc
                    >> !assign_list_nocalc
                    >> keyword_p(";")
                    >> block_nocalc
                ;

            assign_list_nocalc
                =   keyword_p("=") >> list_nocalc
                ;

            arglist_nocalc
                =   keyword_p("(") >> lol_nocalc >> keyword_p(")")
                ;

            rule_nocalc
                =   keyword_p("{") >> block_nocalc >> keyword_p("}")
                |   keyword_p("include") >> list_nocalc >> keyword_p(";")
                |   arg_p >> lol_nocalc >> keyword_p(";")
                |   arg_nocalc >> assign >> list_nocalc >> keyword_p(";")
                |   arg_nocalc >> keyword_p("on") >> list_nocalc
                    >> assign >> list_nocalc >> keyword_p(";")
                |   keyword_p("return") >> list_nocalc >> keyword_p(";")
                |   keyword_p("for") >> !keyword_p("local") >> arg_p
                    >> keyword_p("in") >> list_nocalc
                    >> keyword_p("{") >> block_nocalc >> keyword_p("}")
                |   keyword_p("switch") >> list_nocalc
                    >> keyword_p("{") >> cases_nocalc >> keyword_p("}")
                |   keyword_p("module") >> list_nocalc
                    >> keyword_p("{") >> block_nocalc >> keyword_p("}") 
                |   keyword_p("class") >> lol_nocalc
                    >> keyword_p("{") >> block_nocalc >> keyword_p("}") 
                |   keyword_p("while") >> expr_nocalc
                    >> keyword_p("{") >> block_nocalc >> keyword_p("}") 
                |   keyword_p("if") >> expr_nocalc
                    >> keyword_p("{") >> block_nocalc >> keyword_p("}")
                    >> !( keyword_p("else") >> rule_nocalc )
                |   !keyword_p("local") >> keyword_p("rule_nocalc") >> arg_p
                    >> !arglist_nocalc >> rule_nocalc
                |   keyword_p("on") >> arg_nocalc >> rule_nocalc
                |   keyword_p("actions") >> eflags_nocalc
                    >> arg_p >> !bindlist_nocalc
                    >> eps_p(keyword_p("{"))
                    >> lexeme_d[ '{' >> string_p ]
                    >> keyword_p("}")
                ;

            expr_nocalc
                =   and_expr_nocalc
                    %   (   keyword_p("|")
                        |   keyword_p("||")
                        )
                ;

            and_expr_nocalc
                =   eq_expr_nocalc
                    %   (   keyword_p("&")
                        |   keyword_p("&&")
                        )
                ;

            eq_expr_nocalc
                =   rel_expr_nocalc
                    %   (   keyword_p("=")
                        |   keyword_p("!=")
                        )
                ;

            rel_expr_nocalc
                =   not_expr_nocalc
                    %   (   keyword_p("<")
                        |   keyword_p("<=")
                        |   keyword_p(">")
                        |   keyword_p(">=")
                        )
                ;

            not_expr_nocalc
                =   *keyword_p("!") >> prim_expr_nocalc
                ;

            prim_expr_nocalc
                =   arg_nocalc >> !(keyword_p("in")  >> list_nocalc)
                |   keyword_p("(") >> expr_nocalc >> keyword_p(")")
                ;

            cases_nocalc
                =   *case_nocalc
                ;

            case_nocalc
                =   keyword_p("case") >> arg_p >> keyword_p(":") >> block_nocalc
                ;

            lol_nocalc
                =   list_nocalc % keyword_p(":")
                ;

            list_nocalc
                = *non_punct_nocalc
                ;

            non_punct_nocalc
                =   non_punct_p
                |   keyword_p("[") >> func_nocalc >> keyword_p("]")
                ;

            arg_nocalc
                =   arg_p
                |   keyword_p("[") >> func_nocalc >> keyword_p("]")
                ;

            func_nocalc
                =   arg_nocalc >> lol_nocalc
                |   keyword_p("on") >> arg_nocalc >> arg_nocalc >> lol_nocalc
                |   keyword_p("on") >> arg_nocalc
                    >> keyword_p("return") >> list_nocalc
                ;

            eflags_nocalc
                = *eflag_nocalc;

            eflag_nocalc
                =   keyword_p("updated")
                |   keyword_p("together")
                |   keyword_p("ignore")
                |   keyword_p("quietly")
                |   keyword_p("piecemeal")
                |   keyword_p("existing")
                ;

            bindlist_nocalc
                =   keyword_p("bind") >> list_nocalc
                ;

            BOOST_SPIRIT_DEBUG_RULE(run);
            BOOST_SPIRIT_DEBUG_RULE(block);
            BOOST_SPIRIT_DEBUG_RULE(rules);
            BOOST_SPIRIT_DEBUG_RULE(assign_list);
            BOOST_SPIRIT_DEBUG_RULE(arglist);
            BOOST_SPIRIT_DEBUG_RULE(rule);
            BOOST_SPIRIT_DEBUG_RULE(set_stmt);
            BOOST_SPIRIT_DEBUG_RULE(assign);
            BOOST_SPIRIT_DEBUG_RULE(rule_stmt);
            BOOST_SPIRIT_DEBUG_RULE(expr);
            BOOST_SPIRIT_DEBUG_RULE(and_expr);
            BOOST_SPIRIT_DEBUG_RULE(eq_expr);
            BOOST_SPIRIT_DEBUG_RULE(rel_expr);
            BOOST_SPIRIT_DEBUG_RULE(not_expr);
            BOOST_SPIRIT_DEBUG_RULE(prim_expr);
            BOOST_SPIRIT_DEBUG_RULE(cases);
            BOOST_SPIRIT_DEBUG_RULE(case_);
            BOOST_SPIRIT_DEBUG_RULE(lol);
            BOOST_SPIRIT_DEBUG_RULE(list);
            BOOST_SPIRIT_DEBUG_RULE(non_punct);
            BOOST_SPIRIT_DEBUG_RULE(arg);
            BOOST_SPIRIT_DEBUG_RULE(func);
            BOOST_SPIRIT_DEBUG_RULE(eflags);
            BOOST_SPIRIT_DEBUG_RULE(eflag);
            BOOST_SPIRIT_DEBUG_RULE(bindlist);
            BOOST_SPIRIT_DEBUG_RULE(block_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(rules_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(assign_list_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(arglist_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(rule_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(and_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(eq_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(rel_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(not_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(prim_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(cases_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(case_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(lol_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(list_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(non_punct_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(arg_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(func_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(eflags_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(eflag_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(bindlist_nocalc);
        }

        const rule_t& start() const { return run; }
    };
};

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP
