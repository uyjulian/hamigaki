// bjam_actions.hpp: actions for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP

#include <hamigaki/bjam/grammars/assign_modes.hpp>
#include <hamigaki/bjam/grammars/bjam_expression_grammar_gen.hpp>
#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <climits> // required for <boost/spirit/phoenix/operators.hpp>
#include <boost/spirit/phoenix.hpp>

namespace hamigaki { namespace bjam {

struct try_front_impl
{
    typedef boost::optional<std::string> result_type;

    boost::optional<std::string> operator()(const list_type& x) const
    {
        if (x.empty())
            return boost::optional<std::string>();
        else
            return *x.begin();
    }
};

const ::phoenix::functor<try_front_impl> try_front = try_front_impl();


struct eval_expr_impl
{
    typedef list_type result_type;

    template<class Iterator>
    list_type operator()(context& ctx, Iterator first, Iterator last) const
    {
        typedef bjam_expression_grammar_gen<Iterator> grammar_type;
        return grammar_type::evaluate(first, last, ctx);
    }
};

const ::phoenix::functor<eval_expr_impl> eval_expr = eval_expr_impl();


struct var_set_impl
{
    typedef void result_type;

    void operator()(
        context& ctx, assign_mode::values mode,
        const list_type& names, const list_type& values) const
    {
        variable_table& table =
            ctx.current_frame().current_module().variables;

        typedef list_type::const_iterator iter_type;

        if (mode == assign_mode::set)
        {
            for (iter_type i = names.begin(), end = names.end(); i != end; ++i)
                table.set_values(*i, values);
        }
        else if (mode == assign_mode::append)
        {
            for (iter_type i = names.begin(), end = names.end(); i != end; ++i)
                table.append_values(*i, values);
        }
        else
        {
            for (iter_type i = names.begin(), end = names.end(); i != end; ++i)
                table.set_default_values(*i, values);
        }
    }
};

const ::phoenix::functor<var_set_impl> var_set = var_set_impl();


struct rule_set_impl
{
    typedef void result_type;

    template<class Iterator>
    void operator()(
        context& ctx, const std::string& name, const list_of_list& params,
        Iterator first, Iterator last, bool exported) const
    {
        frame& f = ctx.current_frame();
        rule_table& table = f.current_module().rules;

        boost::shared_ptr<rule_definition> def(new rule_definition);
        def->parameters = params;
        def->body.reset(new std::string(first, last));
        def->module_name = f.module_name();
        def->exported = exported;

        table.set_rule_definition(name, def);
    }
};

const ::phoenix::functor<rule_set_impl> rule_set = rule_set_impl();


struct for_block_impl
{
    typedef void result_type;

    template<class Iterator>
    void operator()(
        context& ctx, const std::string& name, const list_type& values,
        Iterator first, Iterator last, bool is_local) const
    {
        typedef bjam_grammar_gen<Iterator> grammar_type;
        typedef list_type::const_iterator iter_type;

        frame& f = ctx.current_frame();
        variable_table& table = f.current_module().variables;

        scoped_swap_values guard(table, name, is_local);
        for (iter_type i = values.begin(); i != values.end(); ++i)
        {
            table.set_values(name, list_type(*i));
            grammar_type::parse_bjam_grammar(first, last, ctx);
        }
    }
};

const ::phoenix::functor<for_block_impl> for_block = for_block_impl();


struct while_block_impl
{
    typedef list_type result_type;

    template<class Iterator>
    list_type operator()(
        context& ctx, const std::string& expr,
        Iterator first, Iterator last) const
    {
        typedef bjam_grammar_gen<Iterator> grammar_type;

        list_type result;
        while (eval_expr_impl()(ctx, expr.c_str(), expr.c_str()+expr.size()))
            result = grammar_type::parse_bjam_grammar(first, last, ctx).values;
        return result;
    }
};

const ::phoenix::functor<while_block_impl> while_block = while_block_impl();

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP
