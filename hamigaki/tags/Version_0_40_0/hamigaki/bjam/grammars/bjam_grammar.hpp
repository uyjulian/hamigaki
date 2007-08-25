// bjam_grammar.hpp: bjam grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/grammars/base_definition.hpp>
#include <hamigaki/bjam/grammars/bjam_actions.hpp>
#include <hamigaki/bjam/grammars/bjam_closures.hpp>
#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/util/eval_in_module.hpp>
#include <hamigaki/bjam/util/eval_with_variables.hpp>
#include <hamigaki/bjam/util/skip_parser.hpp>
#include <hamigaki/bjam/util/string_parser.hpp>
#include <hamigaki/iterator/line_counting_iterator.hpp>
#include <boost/spirit/dynamic/if.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

namespace hamigaki { namespace bjam {

struct bjam_grammar
    : boost::spirit::grammar<bjam_grammar, list_closure::context_t>
{
    bjam_grammar(bjam::context& ctx) : context(ctx)
    {
    }

    bjam::context& context;

    template<class ScannerT>
    struct definition : base_definition<ScannerT>
    {
        typedef base_definition<ScannerT> base_type;
        typedef typename base_type::rule_t rule_t;
        typedef typename base_type::list_rule_t list_rule_t;
        typedef typename base_type::lol_rule_t lol_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename local_set_stmt_closure::context_t
        > local_set_stmt_rule_t;

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
            typename set_on_stmt_closure::context_t
        > set_on_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename for_stmt_closure::context_t
        > for_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename switch_stmt_closure::context_t
        > switch_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename cases_closure::context_t
        > cases_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename module_stmt_closure::context_t
        > module_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename class_stmt_closure::context_t
        > class_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename while_stmt_closure::context_t
        > while_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename rule_stmt_closure::context_t
        > rule_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename on_stmt_closure::context_t
        > on_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename actions_stmt_closure::context_t
        > actions_stmt_rule_t;

        typedef boost::spirit::rule<
            ScannerT,
            typename eflags_closure::context_t
        > eflags_rule_t;

        using base_type::list;
        using base_type::lol;
        using base_type::arg;
        using base_type::expr_nocalc;
        using base_type::list_nocalc;
        using base_type::lol_nocalc;
        using base_type::arg_nocalc;

        rule_t run;
        list_rule_t block, rules, assign_list;
        local_set_stmt_rule_t local_set_stmt;
        lol_rule_t arglist;
        list_rule_t rule;
        rule_t include_stmt;
        invoke_stmt_rule_t invoke_stmt;
        set_stmt_rule_t set_stmt;
        set_on_stmt_rule_t set_on_stmt;
        assign_rule_t assign;
        for_stmt_rule_t for_stmt;
        switch_stmt_rule_t switch_stmt;
        cases_rule_t cases;
        module_stmt_rule_t module_stmt;
        class_stmt_rule_t class_stmt;
        while_stmt_rule_t while_stmt;
        list_rule_t if_stmt;
        rule_stmt_rule_t rule_stmt;
        on_stmt_rule_t on_stmt;
        actions_stmt_rule_t actions_stmt;
        eflags_rule_t eflags;
        list_rule_t bindlist;

        rule_t block_nocalc, rules_nocalc, assign_list_nocalc;
        rule_t arglist_nocalc, rule_nocalc;
        rule_t cases_nocalc;
        rule_t eflags_nocalc, bindlist_nocalc;

        definition(const bjam_grammar& self)
            : base_definition<ScannerT>(self.context)
        {
            namespace hp = hamigaki::phoenix;
            using namespace boost::spirit;
            using namespace ::phoenix;

            const actor<variable<bjam::context> > ctx(self.context);

            run
                =   !rules [self.values = arg1]
                    >> end_p
                ;

            block
                =   !rules [block.values = arg1]
                ;

            rules
                =   rule [rules.values = arg1]
                    >> !rules [rules.values = arg1]
                |   local_set_stmt [rules.values = arg1]
                ;

            local_set_stmt
                =   keyword_p("local")
                    >> list [local_set_stmt.names = arg1]
                    >> !assign_list [local_set_stmt.values = arg1]
                    >> keyword_p(";")
                    >> eval_with_variables_d
                    (
                        self.context,
                        local_set_stmt.names, local_set_stmt.values
                    )
                    [
                        block [local_set_stmt.values = arg1]
                    ]
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
                |   include_stmt
                |   invoke_stmt [rule.values = arg1]
                |   set_stmt [rule.values = arg1]
                |   set_on_stmt [rule.values = arg1]
                |   keyword_p("return")
                    >> list [rule.values = arg1]
                    >> keyword_p(";")
                |   for_stmt
                |   switch_stmt [rule.values = arg1]
                |   module_stmt [rule.values = arg1]
                |   class_stmt
                |   while_stmt [rule.values = arg1]
                |   if_stmt [rule.values = arg1]
                |   rule_stmt
                |   on_stmt [rule.values = arg1]
                |   actions_stmt
                ;

            include_stmt
                =   keyword_p("include")
                    >> list [include(ctx, arg1)]
                    >> keyword_p(";")
                ;

            invoke_stmt
                =   eps_p [invoke_stmt.caller_line = get_line(arg1)]
                    >> arg_p [invoke_stmt.values = var_expand(ctx, arg1)]
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
                        set_caller_line(ctx, invoke_stmt.caller_line),
                        invoke_stmt.values =
                            invoke_rule(ctx, invoke_stmt.name, invoke_stmt.args)
                    ]
                ;

            set_stmt
                =   arg [set_stmt.names = arg1]
                    >> assign [set_stmt.mode = arg1]
                    >> list [set_stmt.values = arg1]
                    >> keyword_p(";")
                    [
                        var_set(
                            ctx, set_stmt.mode,
                            set_stmt.names, set_stmt.values
                        )
                    ]
                ;

            set_on_stmt
                =   arg [set_on_stmt.names = arg1]
                    >> keyword_p("on")
                    >> list [set_on_stmt.targets = arg1]
                    >> assign [set_on_stmt.mode = arg1]
                    >> list [set_on_stmt.values = arg1]
                    >> keyword_p(";")
                    [
                        var_set_on(
                            ctx,
                            set_on_stmt.mode, set_on_stmt.targets,
                            set_on_stmt.names, set_on_stmt.values
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
                            ctx,
                            for_stmt.name, for_stmt.values,
                            arg1, arg2, for_stmt.is_local
                        )
                    ]
                    >> keyword_p("}")
                ;

            switch_stmt
                =   keyword_p("switch")
                    >> list [switch_stmt.value = force_front(arg1)]
                    >> keyword_p("{")
                    >> cases [switch_stmt.values = arg1]
                    >> keyword_p("}")
                ;

            cases
                =  !(   keyword_p("case")
                        >> arg_p [cases.pattern = arg1]
                        >> keyword_p(":")
                        >> if_p(case_match(cases.pattern, switch_stmt.value))
                        [
                            block [cases.values = arg1]
                            >> cases_nocalc
                        ]
                        .else_p
                        [
                            block_nocalc
                            >> cases [cases.values = arg1]
                        ]
                    )
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

            class_stmt
                =   keyword_p("class")
                    >> lol
                    [
                        class_stmt.module_name = create_class(ctx, arg1)
                    ]
                    >> keyword_p("{")
                    >> eval_in_module_d(self.context, class_stmt.module_name)
                    [
                        block
                    ]
                    >> keyword_p("}")
                ;

            while_stmt
                =   keyword_p("while")
                    >> eps_p [while_stmt.expr_line = get_line(arg1)]
                    >> expr_nocalc
                    [
                        while_stmt.expr = construct_<std::string>(arg1, arg2)
                    ]
                    >> keyword_p("{")
                    >> block_nocalc
                    [
                        while_stmt.values =
                            while_block(
                                ctx, while_stmt.expr, while_stmt.expr_line,
                                arg1, arg2
                            )
                    ]
                    >> keyword_p("}")
                ;

            if_stmt
                =   keyword_p("if")
                    >> expr_nocalc [if_stmt.values = eval_expr(ctx, arg1, arg2)]
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
                            ctx, rule_stmt.name, rule_stmt.params,
                            arg1, arg2, rule_stmt.exported
                        )
                    ]
                ;

            on_stmt
                =   keyword_p("on")
                    >> arg [on_stmt.targets = arg1]
                    >> if_p(hp::empty(on_stmt.targets))
                    [
                        rule_nocalc
                    ]
                    .else_p
                    [
                        eval_on_target_d
                        (
                            self.context, hp::front(on_stmt.targets)
                        )
                        [
                            rule [on_stmt.values = arg1]
                        ]
                    ]
                ;

            actions_stmt
                =   keyword_p("actions")
                    >> eflags [actions_stmt.modifiers = arg1]
                    >> arg_p [actions_stmt.name = arg1]
                    >> !bindlist [actions_stmt.binds = arg1]
                    >> eps_p(keyword_p("{"))
                    >> lexeme_d
                    [
                        '{'
                        >> string_p
                        [
                            actions_set(
                                ctx, actions_stmt.name, arg1,
                                actions_stmt.modifiers,
                                actions_stmt.binds
                            )
                        ]
                    ]
                    >> keyword_p("}")
                ;

            eflags
                =   eps_p
                    [
                        eflags.values = static_cast<action_modifier::values>(0)
                    ]
                    >> *(   keyword_p("updated")
                            [
                                eflags.values =
                                    eflags.values | action_modifier::updated
                            ]
                        |   keyword_p("together")
                            [
                                eflags.values =
                                    eflags.values | action_modifier::together
                            ]
                        |   keyword_p("ignore")
                            [
                                eflags.values =
                                    eflags.values | action_modifier::ignore
                            ]
                        |   keyword_p("quietly")
                            [
                                eflags.values =
                                    eflags.values | action_modifier::quietly
                            ]
                        |   keyword_p("piecemeal")
                            [
                                eflags.values =
                                    eflags.values | action_modifier::piecemeal
                            ]
                        |   keyword_p("existing")
                            [
                                eflags.values =
                                    eflags.values | action_modifier::existing
                            ]
                        )
                ;

            bindlist
                =   keyword_p("bind") >> list [ bindlist.values = arg1 ]
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
                |   !keyword_p("local") >> keyword_p("rule") >> arg_p
                    >> !arglist_nocalc >> rule_nocalc
                |   keyword_p("on") >> arg_nocalc >> rule_nocalc
                |   keyword_p("actions") >> eflags_nocalc
                    >> arg_p >> !bindlist_nocalc
                    >> eps_p(keyword_p("{"))
                    >> lexeme_d[ '{' >> string_p ]
                    >> keyword_p("}")
                ;

            cases_nocalc
                =  *(   keyword_p("case")
                        >> arg_p
                        >> keyword_p(":")
                        >> block_nocalc
                    )
                ;

            eflags_nocalc
                =  *(   keyword_p("updated")
                    |   keyword_p("together")
                    |   keyword_p("ignore")
                    |   keyword_p("quietly")
                    |   keyword_p("piecemeal")
                    |   keyword_p("existing")
                    )
                ;

            bindlist_nocalc
                =   keyword_p("bind") >> list_nocalc
                ;

            BOOST_SPIRIT_DEBUG_RULE(run);
            BOOST_SPIRIT_DEBUG_RULE(block);
            BOOST_SPIRIT_DEBUG_RULE(rules);
            BOOST_SPIRIT_DEBUG_RULE(local_set_stmt);
            BOOST_SPIRIT_DEBUG_RULE(assign_list);
            BOOST_SPIRIT_DEBUG_RULE(arglist);
            BOOST_SPIRIT_DEBUG_RULE(rule);
            BOOST_SPIRIT_DEBUG_RULE(invoke_stmt);
            BOOST_SPIRIT_DEBUG_RULE(set_stmt);
            BOOST_SPIRIT_DEBUG_RULE(set_on_stmt);
            BOOST_SPIRIT_DEBUG_RULE(assign);
            BOOST_SPIRIT_DEBUG_RULE(for_stmt);
            BOOST_SPIRIT_DEBUG_RULE(switch_stmt);
            BOOST_SPIRIT_DEBUG_RULE(cases);
            BOOST_SPIRIT_DEBUG_RULE(module_stmt);
            BOOST_SPIRIT_DEBUG_RULE(while_stmt);
            BOOST_SPIRIT_DEBUG_RULE(if_stmt);
            BOOST_SPIRIT_DEBUG_RULE(rule_stmt);
            BOOST_SPIRIT_DEBUG_RULE(on_stmt);
            BOOST_SPIRIT_DEBUG_RULE(eflags);
            BOOST_SPIRIT_DEBUG_RULE(bindlist);
            BOOST_SPIRIT_DEBUG_RULE(block_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(rules_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(assign_list_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(arglist_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(rule_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(cases_nocalc);
            BOOST_SPIRIT_DEBUG_RULE(eflags_nocalc);
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
parse_info<IteratorT>
bjam_grammar_gen<IteratorT>::parse_bjam_grammar(
    const IteratorT& first, const IteratorT& last, context& ctx, int line)
{
    using namespace ::phoenix;

    bjam::bjam_grammar g(ctx);
    bjam::skip_parser skip;

    typedef hamigaki::line_counting_iterator<IteratorT> iter_type;

    iter_type beg(first, line);
    iter_type end(last);

    bjam::parse_info<IteratorT> result;

    boost::spirit::parse_info<iter_type> info =
        boost::spirit::parse(beg, end, g[var(result.values) = arg1], skip);

    result.stop = info.stop.base();
    result.hit = info.hit;
    result.full = info.full;
    result.length = info.length;

    return result;
}

#undef HAMIGAKI_BJAM_GRAMMAR_GEN_INLINE

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_GRAMMAR_HPP
