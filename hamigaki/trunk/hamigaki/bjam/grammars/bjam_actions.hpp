// bjam_actions.hpp: actions for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP

#include <hamigaki/bjam/grammars/bjam_expression_grammar_gen.hpp>
#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/util/class.hpp>
#include <hamigaki/bjam/util/pattern.hpp>
#include <hamigaki/bjam/util/search.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <climits> // required for <boost/spirit/phoenix/operators.hpp>
#include <boost/spirit/phoenix.hpp>
#include <fstream>
#include <iterator>

namespace hamigaki { namespace bjam {

struct try_front_impl
{
    typedef boost::optional<std::string> result_type;

    boost::optional<std::string> operator()(const string_list& x) const
    {
        if (x.empty())
            return boost::optional<std::string>();
        else
            return *x.begin();
    }
};

const ::phoenix::functor<try_front_impl> try_front = try_front_impl();


struct force_front_impl
{
    typedef std::string result_type;

    std::string operator()(const string_list& x) const
    {
        if (x.empty())
            return std::string();
        else
            return *x.begin();
    }
};

const ::phoenix::functor<force_front_impl> force_front = force_front_impl();


template<class Iterator>
inline string_list evaluate_expr(
    context& ctx, Iterator first, Iterator last, int line)
{
    typedef bjam_expression_grammar_gen<Iterator> grammar_type;
    return grammar_type::evaluate(first, last, ctx, line);
}

struct eval_expr_impl
{
    typedef string_list result_type;

    template<class Iterator>
    string_list operator()(context& ctx, Iterator first, Iterator last) const
    {
        return evaluate_expr(ctx, first.base(), last.base(), first.line());
    }
};

const ::phoenix::functor<eval_expr_impl> eval_expr = eval_expr_impl();


struct include_impl
{
    typedef void result_type;

    void operator()(context& ctx, const string_list& names) const
    {
        typedef bjam_grammar_gen<const char*> grammar_type;

        if (names.empty())
            return;

        const std::string& target_name = names[0];
        scoped_on_target gurad(ctx, target_name);
        const std::string& filename = search_target(ctx, target_name);

        std::string str;
        {
            std::ifstream is(filename.c_str(), std::ios_base::binary);
            if (!is)
                throw cannot_open_file(filename);

            str.assign(
                std::istreambuf_iterator<char>(is),
                (std::istreambuf_iterator<char>())
            );
        }

        const char* first = str.c_str();
        const char* last = first + str.size();

        scoped_change_filename guard(ctx.current_frame(), filename);
        grammar_type::parse_bjam_grammar(first, last, ctx, 1);
    }
};

const ::phoenix::functor<include_impl> include = include_impl();


struct var_set_impl
{
    typedef void result_type;

    void operator()(
        context& ctx, assign_mode::values mode,
        const string_list& names, const string_list& values) const
    {
        variable_table& table =
            ctx.current_frame().current_module().variables;

        set_variables(table, mode, names, values);
    }
};

const ::phoenix::functor<var_set_impl> var_set = var_set_impl();


struct var_set_on_impl
{
    typedef void result_type;

    void operator()(
        context& ctx,
        assign_mode::values mode, const string_list& targets,
        const string_list& names, const string_list& values) const
    {
        if (mode != assign_mode::append)
            mode = assign_mode::set;

        typedef string_list::const_iterator iter_type;

        for (iter_type i = targets.begin(), end = targets.end(); i != end; ++i)
        {
            variable_table& table = ctx.get_target(*i).variables;
            set_variables(table, mode, names, values);
        }
    }
};

const ::phoenix::functor<var_set_on_impl> var_set_on = var_set_on_impl();


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

        rule_definition def;
        def.parameters = params;
        def.body.reset(new std::string(first, last));
        def.module_name = f.module_name();
        def.exported = exported;
        def.filename = f.filename();
        def.line = first.line();

        table.set_rule_body(name, def);
    }
};

const ::phoenix::functor<rule_set_impl> rule_set = rule_set_impl();


struct for_block_impl
{
    typedef void result_type;

    template<class Iterator>
    void operator()(
        context& ctx, const std::string& name, const string_list& values,
        Iterator first, Iterator last, bool is_local) const
    {
        typedef typename Iterator::base_type base_iterator;
        typedef bjam_grammar_gen<base_iterator> grammar_type;
        typedef string_list::const_iterator iter_type;

        frame& f = ctx.current_frame();
        variable_table& table = f.current_module().variables;

        int line = first.line();

        scoped_swap_values guard(table, name, is_local);
        for (iter_type i = values.begin(); i != values.end(); ++i)
        {
            table.set_values(name, string_list(*i));
            grammar_type::parse_bjam_grammar(
                first.base(), last.base(), ctx, line);
        }
    }
};

const ::phoenix::functor<for_block_impl> for_block = for_block_impl();


struct create_class_impl
{
    typedef std::string result_type;

    std::string operator()(context& ctx, const list_of_list& lol) const
    {
        if (lol[0].empty())
            throw std::runtime_error("missing class name"); // FIXME

        return make_class(ctx, lol[0][0], lol[1]);
    }
};

const ::phoenix::functor<create_class_impl> create_class = create_class_impl();


struct while_block_impl
{
    typedef string_list result_type;

    template<class Iterator>
    string_list operator()(
        context& ctx, const std::string& expr, int expr_line,
        Iterator first, Iterator last) const
    {
        typedef typename Iterator::base_type base_iterator;
        typedef bjam_grammar_gen<base_iterator> grammar_type;

        int block_line = first.line();

        string_list result;
        while (evaluate_expr(
            ctx, expr.c_str(), expr.c_str()+expr.size(), expr_line))
        {
            result =
                grammar_type::parse_bjam_grammar(
                    first.base(), last.base(), ctx, block_line
                ).values;
        }
        return result;
    }
};

const ::phoenix::functor<while_block_impl> while_block = while_block_impl();


struct case_match_impl
{
    typedef bool result_type;

    bool operator()(const std::string& pattern, const std::string& value) const
    {
        return pattern_match(pattern, value);
    }
};

const ::phoenix::functor<case_match_impl> case_match = case_match_impl();


struct actions_set_impl
{
    typedef void result_type;

    void operator()(
        context& ctx, const std::string& name, const std::string& commands,
        action_modifier::values modifiers, const string_list& binds) const
    {
        frame& f = ctx.current_frame();
        rule_table& table = f.current_module().rules;

        rule_definition def;
        def.commands = commands;
        def.modifiers = modifiers;
        def.binds = binds;

        table.set_rule_actions(name, def);

        const boost::optional<std::string> module_name = f.module_name();
        if (module_name)
        {
            std::string full_name = *module_name;
            full_name += '.';
            full_name += name;

            module& root = ctx.get_module(boost::optional<std::string>());
            root.rules.set_rule_actions(full_name, def);
        }
    }
};

const ::phoenix::functor<actions_set_impl> actions_set = actions_set_impl();

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP
