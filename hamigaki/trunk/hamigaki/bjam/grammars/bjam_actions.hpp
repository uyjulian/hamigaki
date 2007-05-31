// bjam_actions.hpp: actions for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP
#define HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP

#include <hamigaki/bjam/grammars/assign_modes.hpp>
#include <hamigaki/bjam/util/variable_expansion.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <climits> // required for <boost/spirit/phoenix/operators.hpp>
#include <boost/spirit/phoenix.hpp>

namespace hamigaki { namespace bjam {

struct set_true_impl
{
    typedef void result_type;

    void operator()(list_type& lhs, const list_type& rhs) const
    {
        if (!lhs)
        {
            if (rhs)
                lhs = rhs;
            else
                lhs = list_type("1");
        }
    }

    void operator()(list_type& lhs) const
    {
        if (!lhs)
            lhs = list_type("1");
    }
};

const ::phoenix::functor<set_true_impl> set_true = set_true_impl();


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


struct includes_impl
{
    typedef bool result_type;

    bool operator()(const list_type& lhs, const list_type& rhs) const
    {
        typedef list_type::const_iterator iter_type;

        iter_type lb = lhs.begin();
        iter_type le = lhs.end();
        iter_type rb = rhs.begin();
        iter_type re = rhs.end();

        for (iter_type i = lb; i != le; ++i)
            if (std::find(rb, re, *i) == re)
                return false;

        return true;
    }
};

const ::phoenix::functor<includes_impl> includes = includes_impl();


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


struct var_expand_impl
{
    typedef void result_type;

    void operator()(
        context& ctx, list_type& result, const std::string& s) const
    {
        frame& f = ctx.current_frame();
        const variable_table& table = f.current_module().variables;
        const list_of_list& args = f.arguments();

        bjam::expand_variable(result, s, table, args);
    }
};

const ::phoenix::functor<var_expand_impl> var_expand = var_expand_impl();


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

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_GRAMMARS_BJAM_ACTIONS_HPP
