// bjam_grammar.hpp: bjam grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/grammars/bjam_actions.hpp>
#include <hamigaki/bjam/grammars/bjam_closures.hpp>
#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/util/argument_parser.hpp>
#include <hamigaki/bjam/util/eval_in_module.hpp>
#include <hamigaki/bjam/util/keyword_parser.hpp>
#include <hamigaki/bjam/util/skip_parser.hpp>
#include <hamigaki/bjam/util/string_parser.hpp>
#include <hamigaki/spirit/phoenix/stl/clear.hpp>
#include <boost/spirit/dynamic/if.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

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
            typename invoke_stmt_closure::context_t
        > invoke_stmt_rule_t;

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
            typename for_stmt_closure::context_t
        > for_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename module_stmt_closure::context_t
        > module_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename while_stmt_closure::context_t
        > while_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename rule_stmt_closure::context_t
        > rule_stmt_rule_t;

        rule_t run;
        list_rule_t block, rules, assign_list;
        lol_rule_t arglist;
        list_rule_t rule;
        invoke_stmt_rule_t invoke_stmt;
        set_stmt_rule_t set_stmt;
        assign_rule_t assign;
        for_stmt_rule_t for_stmt;
        module_stmt_rule_t module_stmt;
        while_stmt_rule_t while_stmt;
        list_rule_t if_stmt;
        rule_stmt_rule_t rule_stmt;
        rule_t cases, case_;
        lol_rule_t lol;
        list_rule_t list, non_punct, arg;
        invoke_stmt_rule_t func;
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
                =   !rules
                    [
                        set_return_values(boost::ref(self.context), arg1)
                    ]
                    >> end_p
                ;

            block
                =   !rules [block.values = arg1]
                ;

            rules
                =   rule [rules.values = arg1]
                    >> !rules [rules.values = arg1]
                |   keyword_p("local")
                    >> list
                    >> !assign_list
                    >> keyword_p(";")
                    >> block [rules.values = arg1]
                ;

            assign_list
                =   keyword_p("=")
                    >> list [assign_list.values = arg1]
                ;

            arglist
                =   keyword_p("(")
                    >> lol [arglist.values = arg1]
                    >> keyword_p(")")
                ;

            rule
                =   keyword_p("{")
                    >> block [rule.values = arg1]
                    >> keyword_p("}")
                |   keyword_p("include") >> list >> keyword_p(";")
                |   invoke_stmt [rule.values = arg1]
                |   set_stmt [rule.values = arg1]
                |   arg >> keyword_p("on") >> list
                    >> assign >> list >> keyword_p(";")
                |   keyword_p("return")
                    >> list [rule.values = arg1]
                    >> keyword_p(";")
                |   for_stmt
                |   keyword_p("switch") >> list
                    >> keyword_p("{") >> cases >> keyword_p("}")
                |   module_stmt
                |   keyword_p("class") >> lol
                    >> keyword_p("{") >> block >> keyword_p("}")
                |   while_stmt [rule.values = arg1]
                |   if_stmt [rule.values = arg1]
                |   rule_stmt
                |   keyword_p("on") >> arg >> rule
                |   keyword_p("actions") >> eflags >> arg_p >> !bindlist
                    >> eps_p(keyword_p("{"))
                    >> lexeme_d[ '{' >> string_p ]
                    >> keyword_p("}")
                ;

            invoke_stmt
                =   arg_p
                    [
                        invoke_stmt.values =
                            var_expand(boost::ref(self.context), arg1)
                    ]
                    >> lol
                    [
                        invoke_stmt.args = arg1,
                        invoke_stmt.name =
                            split_rule_name(
                                invoke_stmt.values, invoke_stmt.args
                            )
                    ]
                    >> keyword_p(";")
                    [
                        invoke_stmt.values =
                            invoke_rule(
                                boost::ref(self.context),
                                invoke_stmt.name, invoke_stmt.args
                            )
                    ]
                ;

            set_stmt
                =   arg [set_stmt.names = arg1]
                    >> assign [set_stmt.mode = arg1]
                    >> list [set_stmt.values = arg1]
                    >> keyword_p(";")
                    [
                        var_set(
                            boost::ref(self.context), set_stmt.mode,
                            set_stmt.names, set_stmt.values
                        )
                    ]
                ;

            assign
                =   keyword_p("=") [assign.values = assign_mode::set]
                |   keyword_p("+=") [assign.values = assign_mode::append]
                |   keyword_p("?=") [assign.values = assign_mode::set_default]
                |   keyword_p("default")
                    >> keyword_p("=")
                    [
                        assign.values = assign_mode::set_default
                    ]
                ;

            for_stmt
                =   keyword_p("for") [for_stmt.is_local = false]
                    >> !keyword_p("local") [for_stmt.is_local = true]
                    >> arg_p [for_stmt.name = arg1]
                    >> keyword_p("in")
                    >> list [for_stmt.values = arg1]
                    >> keyword_p("{")
                    >> block_nocalc
                    [
                        for_block(
                            boost::ref(self.context),
                            for_stmt.name, for_stmt.values,
                            arg1, arg2, for_stmt.is_local
                        )
                    ]
                    >> keyword_p("}")
                ;

            module_stmt
                =   keyword_p("module")
                    >> list [module_stmt.name = try_front(arg1)]
                    >> keyword_p("{")
                    >> eval_in_module_d(self.context, module_stmt.name)
                    [
                        block [module_stmt.values = arg1]
                    ]
                    >> keyword_p("}")
                ;

            while_stmt
                =   keyword_p("while")
                    >> expr_nocalc [while_stmt.expr = arg1]
                    >> keyword_p("{")
                    >> block_nocalc
                    [
                        while_block(
                            boost::ref(self.context),
                            while_stmt.expr, arg1, arg2
                        )
                    ]
                    >> keyword_p("}")
                    [
                        while_stmt.values =
                            get_return_values(boost::ref(self.context))
                    ]
                ;

            if_stmt
                =   keyword_p("if")
                    >> expr_nocalc
                    [
                        if_stmt.values =
                            eval_expr(boost::ref(self.context), arg1, arg2)
                    ]
                    >> if_p(if_stmt.values)
                    [
                        keyword_p("{")
                        >> block [if_stmt.values = arg1]
                        >> keyword_p("}")
                        >> !( keyword_p("else") >> rule_nocalc )
                    ]
                    .else_p
                    [
                        keyword_p("{")
                        >> block_nocalc
                        >> keyword_p("}")
                        >> !(   keyword_p("else")
                                >> rule [if_stmt.values = arg1]
                            )
                    ]
                ;

            rule_stmt
                =   eps_p [rule_stmt.exported = true]
                    >> !keyword_p("local") [rule_stmt.exported = false]
                    >> keyword_p("rule")
                    >> arg_p [rule_stmt.name = arg1]
                    >> !arglist [rule_stmt.params = arg1]
                    >> rule_nocalc
                    [
                        rule_set(
                            boost::ref(self.context),
                            rule_stmt.name, rule_stmt.params,
                            arg1, arg2, rule_stmt.exported
                        )
                    ]
                ;

            cases
                =   *case_
                ;

            case_
                =   keyword_p("case") >> arg_p >> keyword_p(":") >> block
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
                        non_punct.values =
                            var_expand(boost::ref(self.context), arg1)
                    ]
                |   keyword_p("[")
                    >> func [non_punct.values = arg1]
                    >> keyword_p("]")
                ;

            arg
                =   arg_p
                    [
                        arg.values = var_expand(boost::ref(self.context), arg1)
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
                                boost::ref(self.context),
                                func.name, func.args
                            )
                    ]
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
            BOOST_SPIRIT_DEBUG_RULE(module_stmt);
            BOOST_SPIRIT_DEBUG_RULE(rule_stmt);
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

#if HAMIGAKI_BJAM_SEPARATE_GRAMMAR_INSTANTIATION != 0
    #define HAMIGAKI_BJAM_GRAMMAR_GEN_INLINE
#else
    #define HAMIGAKI_BJAM_GRAMMAR_GEN_INLINE inline
#endif 

template<class IteratorT>
HAMIGAKI_BJAM_GRAMMAR_GEN_INLINE
boost::spirit::parse_info<IteratorT>
bjam_grammar_gen<IteratorT>::parse_bjam_grammar(
    const IteratorT& first, const IteratorT& last, context& ctx)
{
    bjam::bjam_grammar g(ctx);
    bjam::skip_parser skip;

    IteratorT current = first;
    return boost::spirit::parse(current, last, g, skip);
}

#undef HAMIGAKI_BJAM_GRAMMAR_GEN_INLINE

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP
