// base_definition.hpp: bjam base grammar definition

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BASE_DEFINITION_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BASE_DEFINITION_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/grammars/base_closures.hpp>
#include <hamigaki/bjam/grammars/base_actions.hpp>
#include <hamigaki/bjam/util/argument_parser.hpp>
#include <hamigaki/bjam/util/keyword_parser.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

template<class ScannerT>
struct base_definition
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
        typename func_closure::context_t
    > func_rule_t;

    lol_rule_t lol;
    list_rule_t list, non_punct, arg;
    func_rule_t func;

    rule_t expr_nocalc, and_expr_nocalc, eq_expr_nocalc;
    rule_t rel_expr_nocalc, not_expr_nocalc, prim_expr_nocalc;
    rule_t lol_nocalc, list_nocalc, non_punct_nocalc, arg_nocalc;
    rule_t func_nocalc;

    base_definition(bjam::context& context)
    {
        using namespace boost::spirit;
        using namespace ::phoenix;

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
                    non_punct.values =
                        var_expand(boost::ref(context), arg1)
                ]
            |   keyword_p("[")
                >> func [non_punct.values = arg1]
                >> keyword_p("]")
            ;

        arg
            =   arg_p
                [
                    arg.values = var_expand(boost::ref(context), arg1)
                ]
            |   keyword_p("[")
                >> func [arg.values = arg1]
                >> keyword_p("]")
            ;

        func
            =   arg [func.values = arg1]
                >> lol
                [
                    func.args = arg1,
                    func.name =
                        split_rule_name(
                            func.values, func.args
                        ),
                    func.values =
                        invoke_rule(
                            boost::ref(context),
                            func.name, func.args
                        )
                ]
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
};

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_GRAMMARS_BASE_DEFINITION_HPP
