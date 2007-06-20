// bjam_expression_grammar.hpp: bjam expression grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_GRAMMAR_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_GRAMMAR_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/grammars/base_definition.hpp>
#include <hamigaki/bjam/grammars/bjam_expression_actions.hpp>
#include <hamigaki/bjam/grammars/bjam_expression_grammar_gen.hpp>
#include <hamigaki/bjam/util/skip_parser.hpp>
#include <hamigaki/iterator/line_counting_iterator.hpp>
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
    struct definition : base_definition<ScannerT>
    {
        typedef base_definition<ScannerT> base_type;
        typedef typename base_type::list_rule_t list_rule_t;

        using base_type::and_expr_nocalc;
        using base_type::eq_expr_nocalc;
        using base_type::arg;
        using base_type::list;
        using base_type::list_nocalc;

        list_rule_t top;
        list_rule_t expr, and_expr, eq_expr, rel_expr, not_expr, prim_expr;

        definition(const bjam_expression_grammar& self)
            : base_definition<ScannerT>(self.context)
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
                                and_expr_nocalc
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

            BOOST_SPIRIT_DEBUG_RULE(top);
            BOOST_SPIRIT_DEBUG_RULE(expr);
            BOOST_SPIRIT_DEBUG_RULE(and_expr);
            BOOST_SPIRIT_DEBUG_RULE(eq_expr);
            BOOST_SPIRIT_DEBUG_RULE(rel_expr);
            BOOST_SPIRIT_DEBUG_RULE(not_expr);
            BOOST_SPIRIT_DEBUG_RULE(prim_expr);
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
string_list bjam_expression_grammar_gen<IteratorT>::evaluate(
    const IteratorT& first, const IteratorT& last, context& ctx, int line)
{
    using namespace ::phoenix;

    bjam::bjam_expression_grammar g(ctx);
    bjam::skip_parser skip;

    typedef hamigaki::line_counting_iterator<IteratorT> iter_type;

    iter_type beg(first, line);
    iter_type end(last);

    string_list result;
    boost::spirit::parse(beg, end, g [var(result) = arg1], skip);
    return result;
}

#undef HAMIGAKI_BJAM_EXPRGRAMMAR_GEN_INLINE

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_EXPRESSION_GRAMMAR_HPP
