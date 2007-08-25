// base_actions.hpp: actions for base_definition

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BASE_ACTIONS_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BASE_ACTIONS_HPP

#include <hamigaki/bjam/util/variable_expansion.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <climits> // required for <boost/spirit/phoenix/operators.hpp>
#include <boost/spirit/phoenix.hpp>
#include <boost/next_prior.hpp>

namespace hamigaki { namespace bjam {

struct var_expand_impl
{
    typedef string_list result_type;

    string_list operator()(context& ctx, const std::string& s) const
    {
        frame& f = ctx.current_frame();
        const variable_table& table = f.current_module().variables;
        const list_of_list& args = f.arguments();

        return bjam::expand_variable(s, table, args);
    }
};

const ::phoenix::functor<var_expand_impl> var_expand = var_expand_impl();


struct split_rule_name_impl
{
    typedef boost::optional<std::string> result_type;

    boost::optional<std::string>
    operator()(const string_list& values, list_of_list& args) const
    {
        if (values.empty())
            return boost::optional<std::string>();

        string_list arg(boost::next(values.begin()), values.end());
        if (args.empty())
            args.push_back(arg);
        else
        {
            list_of_list tmp;
            arg += args[0];
            tmp.push_back(arg);
            for (std::size_t i = 1, size = args.size(); i < size; ++i)
                tmp.push_back(args[i]);
            args.swap(tmp);
        }

        return values[0];
    }
};

const ::phoenix::functor<
    split_rule_name_impl
> split_rule_name = split_rule_name_impl();


struct get_line_impl
{
    typedef int result_type;

    template<class Iterator>
    int operator()(Iterator it) const
    {
        return it.line();
    }
};

const ::phoenix::functor<get_line_impl> get_line = get_line_impl();


struct set_caller_line_impl
{
    typedef void result_type;

    void operator()(context& ctx, int line) const
    {
        ctx.current_frame().line(line);
    }
};

const ::phoenix::functor<
    set_caller_line_impl
> set_caller_line = set_caller_line_impl();


struct invoke_rule_impl
{
    typedef string_list result_type;

    string_list operator()(
        context& ctx, const std::string& name, const list_of_list& args) const
    {
        return ctx.invoke_rule(name, args);
    }

    string_list operator()(
        context& ctx,
        const boost::optional<std::string>& name,
        const list_of_list& args) const
    {
        if (name)
            return ctx.invoke_rule(*name, args);
        else
            return string_list();
    }
};

const ::phoenix::functor<invoke_rule_impl> invoke_rule = invoke_rule_impl();

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BASE_ACTIONS_HPP
