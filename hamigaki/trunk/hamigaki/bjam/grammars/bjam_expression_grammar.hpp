// bjam_expression_grammar.hpp: bjam expression grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_GRAMMAR_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_GRAMMAR_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/grammars/bjam_actions.hpp>
#include <hamigaki/bjam/grammars/bjam_closures.hpp>
#include <hamigaki/bjam/grammars/bjam_expression_grammar_gen.hpp>
#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/util/argument_parser.hpp>
#include <hamigaki/bjam/util/keyword_parser.hpp>
#include <hamigaki/bjam/util/skip_parser.hpp>
#include <hamigaki/bjam/util/string_parser.hpp>
#include <hamigaki/spirit/phoenix/stl/clear.hpp>
#include <boost/spirit/dynamic/if.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

struct bjam_expression_grammar
    : boost::spirit::grammar<bjam_expression_grammar, list_closure::context_t>
{
    bjam_expression_grammar(bjam::context& ctx) : context(ctx)
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

        list_rule_t top;
        list_rule_t expr, and_expr, eq_expr, rel_expr, not_expr, prim_expr;
        lol_rule_t lol;
        list_rule_t list, non_punct, arg, func;

        rule_t expr_nocalc, and_expr_nocalc, eq_expr_nocalc;
        rule_t rel_expr_nocalc, not_expr_nocalc, prim_expr_nocalc;
        rule_t lol_nocalc, list_nocalc, non_punct_nocalc, arg_nocalc;
        rule_t func_nocalc;

        definition(const bjam_expression_grammar& self)
        {
            namespace hp = hamigaki::phoenix;
            using namespace boost::spirit;
            using namespace ::phoenix;

            top
                =   expr [self.values = arg1]
                ;

            expr
                =   and_expr [expr.values = arg1]
                    >> *(
                            ( keyword_p("|") | keyword_p("||") )
                            >> if_p(expr.values)
                            [
                                and_expr_nocalc [hp::clear(expr.values)]
                            ]
                            .else_p
                            [
                                and_expr [expr.values = arg1]
                            ]
                        )
                ;

            and_expr
                =   eq_expr [and_expr.values = arg1]
                    >> *(
                            ( keyword_p("&") | keyword_p("&&") )
                            >> if_p(and_expr.values)
                            [
                                eq_expr
                                [
                                    if_(!arg1)
                                    [
                                        hp::clear(and_expr.values)
                                    ]
                                ]
                            ]
                            .else_p
                            [
                                eq_expr_nocalc [hp::clear(and_expr.values)]
                            ]
                        )
                ;

            eq_expr
                =   rel_expr [eq_expr.values = arg1]
                    >> *(   keyword_p("=")
                            >> rel_expr
                            [
                                if_(eq_expr.values == arg1)
                                [
                                    set_true(eq_expr.values, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(eq_expr.values)
                                ]
                            ]
                        |   keyword_p("!=")
                            >> rel_expr
                            [
                                if_(eq_expr.values != arg1)
                                [
                                    set_true(eq_expr.values, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(eq_expr.values)
                                ]
                            ]
                        )
                ;

            rel_expr
                =   not_expr [rel_expr.values = arg1]
                    >> *(   keyword_p("<")
                            >> not_expr
                            [
                                if_(rel_expr.values < arg1)
                                [
                                    set_true(rel_expr.values, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(rel_expr.values)
                                ]
                            ]
                        |   keyword_p("<=")
                            >> not_expr
                            [
                                if_(rel_expr.values <= arg1)
                                [
                                    set_true(rel_expr.values, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(rel_expr.values)
                                ]
                            ]
                        |   keyword_p(">")
                            >> not_expr
                            [
                                if_(rel_expr.values > arg1)
                                [
                                    set_true(rel_expr.values, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(rel_expr.values)
                                ]
                            ]
                        |   keyword_p(">=")
                            >> not_expr
                            [
                                if_(rel_expr.values >= arg1)
                                [
                                    set_true(rel_expr.values, arg1)
                                ]
                                .else_
                                [
                                    hp::clear(rel_expr.values)
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
                            hp::clear(not_expr.values)
                        ]
                        .else_
                        [
                            set_true(not_expr.values, arg1)
                        ]
                    ]
                |   prim_expr [not_expr.values = arg1]
                ;

            prim_expr
                =   arg [prim_expr.values = arg1]
                    >> !(   keyword_p("in")
                            >> if_p(prim_expr.values)
                            [
                                list
                                [
                                    if_(includes(prim_expr.values, arg1))
                                    [
                                        set_true(prim_expr.values, arg1)
                                    ]
                                    .else_
                                    [
                                        hp::clear(prim_expr.values)
                                    ]
                                ]
                            ]
                            .else_p
                            [
                                list_nocalc
                                [
                                    set_true(prim_expr.values)
                                ]
                            ]
                        )
                |   keyword_p("(")
                    >> expr [prim_expr.values = arg1]
                    >> keyword_p(")")
                ;

            lol
                =   list [lol.values += arg1]
                    % keyword_p(":")
                ;

            list
                =   *non_punct [list.values += arg1]
                ;

            non_punct
                =   non_punct_p
                    [
                        var_expand(
                            boost::ref(self.context), non_punct.values, arg1)
                    ]
                |   keyword_p("[")
                    >> func [non_punct.values = arg1]
                    >> keyword_p("]")
                ;

            arg
                =   arg_p
                    [
                        var_expand(boost::ref(self.context), arg.values, arg1)
                    ]
                |   keyword_p("[")
                    >> func [arg.values = arg1]
                    >> keyword_p("]")
                ;

            func
                =   arg >> lol
                |   keyword_p("on") >> arg >> arg >> lol
                |   keyword_p("on") >> arg >> keyword_p("return") >> list
                ;


            // "nocalc" versions

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

            BOOST_SPIRIT_DEBUG_RULE(top);
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
            BOOST_SPIRIT_DEBUG_RULE(expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(and_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(eq_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(rel_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(not_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(prim_expr_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(lol_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(list_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(non_punct_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(arg_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(func_nocalc);
        }

        const list_rule_t& start() const { return top; }
    };
};

#if HAMIGAKI_BJAM_SEPARATE_GRAMMAR_INSTANTIATION != 0
    #define HAMIGAKI_BJAM_EXPRGRAMMAR_GEN_INLINE
#else
    #define HAMIGAKI_BJAM_EXPRGRAMMAR_GEN_INLINE inline
#endif 

template<class IteratorT>
HAMIGAKI_BJAM_EXPRGRAMMAR_GEN_INLINE
list_type bjam_expression_grammar_gen<IteratorT>::evaluate(
    const IteratorT& first, const IteratorT& last, context& ctx)
{
    using namespace ::phoenix;

    bjam::bjam_expression_grammar g(ctx);
    bjam::skip_parser skip;

    IteratorT current = first;
    list_type result;
    boost::spirit::parse(current, last, g [var(result) = arg1], skip);
    return result;
}

#undef HAMIGAKI_BJAM_EXPRGRAMMAR_GEN_INLINE

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_GRAMMAR_HPP
