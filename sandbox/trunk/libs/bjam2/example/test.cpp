// test.cpp: test program for Hamigaki.Bjam

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam2/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam2/grammars/bjam_grammar_id.hpp>
#include <hamigaki/bjam2/util/class.hpp>
#include <hamigaki/bjam2/util/list_of_list.hpp>
#include <hamigaki/bjam2/util/pattern.hpp>
#include <hamigaki/bjam2/util/variable_expansion.hpp>
#include <hamigaki/bjam2/bjam_context.hpp>
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <boost/ref.hpp>
#include <cassert>
#include <exception>
#include <iostream>

namespace bj = hamigaki::bjam2;
namespace bs = boost::spirit;

typedef bs::tree_match<const char*> parse_tree_match_t;
typedef parse_tree_match_t::node_t node_t;
typedef parse_tree_match_t::const_tree_iterator iter_t;

std::string node_value(const node_t& node)
{
    return std::string(node.value.begin(), node.value.end());
}

void set_true(bj::string_list& lhs, const bj::string_list& rhs)
{
    if (lhs)
        return;

    if (rhs)
        lhs = rhs;
    else
        lhs = bj::string_list("1");
}

bj::list_of_list
concatenate_args(const bj::string_list& args0, const bj::list_of_list& args)
{
    if (args0.size() == 1)
        return args;

    bj::string_list arg(boost::next(args0.begin()), args0.end());

    bj::list_of_list tmp;
    arg += args[0];
    tmp.push_back(arg);
    for (std::size_t i = 1, size = args.size(); i < size; ++i)
        tmp.push_back(args[i]);
    return tmp;
}

bool includes(const bj::string_list& lhs, const bj::string_list& rhs)
{
    typedef bj::string_list::const_iterator iter_type;

    iter_type lb = lhs.begin();
    iter_type le = lhs.end();
    iter_type rb = rhs.begin();
    iter_type re = rhs.end();

    for (iter_type i = lb; i != le; ++i)
        if (std::find(rb, re, *i) == re)
            return false;

    return true;
}

bj::string_list eval_arg(bj::context& ctx, const node_t& tree);
bj::string_list eval_list(bj::context& ctx, const node_t& tree);
bj::list_of_list eval_lol(bj::context& ctx, const node_t& tree);
bj::string_list eval_expr(bj::context& ctx, const node_t& tree);
bj::string_list eval_rule(bj::context& ctx, const node_t& tree);
bj::string_list eval_block(bj::context& ctx, const node_t& tree);

bj::string_list eval_func_impl(bj::context& ctx, iter_t beg, iter_t end)
{
    const bj::string_list& args0 = ::eval_arg(ctx, *(beg++));
    bj::list_of_list args;
    if (beg != end)
        args = ::eval_lol(ctx, *beg);

    if (!args0.empty())
        return ctx.invoke_rule(args0[0], concatenate_args(args0, args));
    else
        return bj::string_list();
}

bj::string_list eval_func(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::func_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    if (beg->value.id() == bj::arg_id)
        return ::eval_func_impl(ctx, beg, end);
    else
    {
        ++beg;
        const bj::string_list& targets = ::eval_arg(ctx, *(beg++));
        if (targets.empty())
            return bj::string_list();

        bj::scoped_on_target guard(ctx, targets[0]);
        if (beg->value.id() == bj::arg_id)
            return ::eval_func_impl(ctx, beg, end);
        else
            return ::eval_list(ctx, *beg);
    }
}

bj::string_list eval_arg(bj::context& ctx, const node_t& tree)
{
    assert(
        (tree.value.id() == bj::arg_id) ||
        (tree.value.id() == bj::non_punct_id)
    );
    assert(!tree.children.empty());

    if (tree.children.size() == 1)
    {
        bj::frame& f = ctx.current_frame();
        const bj::variable_table& table = f.current_module().variables;
        const bj::list_of_list& args = f.arguments();

        const std::string& name = ::node_value(tree.children.front());
        return bj::expand_variable(name, table, args);
    }
    else
    {
        return ::eval_func(ctx, tree.children[1]);
    }
}

bj::string_list eval_list(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::list_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bj::string_list list;
    for ( ; beg != end; ++beg)
    {
        if (beg->value.id().to_long() == bj::non_punct_id)
            list += ::eval_arg(ctx, *beg);
    }
    return list;
}

bj::list_of_list eval_lol(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::lol_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bj::list_of_list values;
    for ( ; beg != end; ++beg)
        values.push_back(::eval_list(ctx, *beg));

    return values;
}

bj::string_list eval_prim_expr(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::prim_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    if (beg->value.id() == bj::arg_id)
    {
        bj::string_list values = ::eval_arg(ctx, *(beg++));
        if (beg != end)
        {
            ++beg;
            if (values.empty())
                return bj::string_list("1");
            else if (beg == end)
                return bj::string_list();
            
            const bj::string_list& rhs = ::eval_list(ctx, *(beg++));
            if (::includes(values, rhs))
                ::set_true(values, rhs);
            else
                values.clear();
        }
        return values;
    }
    else
        return ::eval_expr(ctx, tree.children[1]);
}

bj::string_list eval_not_expr(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::not_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bool not_op = false;
    while (beg->value.id() != bj::prim_expr_id)
    {
        not_op = !not_op;
        ++beg;
    }

    const bj::string_list& values = ::eval_prim_expr(ctx, *(beg++));

    if (not_op)
    {
        if (values)
            return bj::string_list();
        else
            return bj::string_list("1");
    }
    else
        return values;
}

bj::string_list eval_rel_expr(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::rel_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bj::string_list values = ::eval_not_expr(ctx, *(beg++));
    while (beg != end)
    {
        node_t::parse_node_t op = (beg++)->value;
        const bj::string_list& rhs = ::eval_not_expr(ctx, *(beg++));

        bool val = false;
        if (*op.begin() == '<')
        {
            if (boost::next(op.begin()) == op.end())
                val = (values < rhs);
            else
                val = (values <= rhs);
        }
        else
        {
            if (boost::next(op.begin()) == op.end())
                val = (values > rhs);
            else
                val = (values >= rhs);
        }

        if (val)
            ::set_true(values, rhs);
        else
            values.clear();
    }

    return values;
}

bj::string_list eval_eq_expr(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::eq_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bj::string_list values = ::eval_rel_expr(ctx, *(beg++));
    while (beg != end)
    {
        bool op = (*(beg++)->value.begin() == '=');
        const bj::string_list& rhs = ::eval_rel_expr(ctx, *(beg++));
        if ((values == rhs) == op)
            ::set_true(values, rhs);
        else
            values.clear();
    }

    return values;
}

bj::string_list eval_and_expr(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::and_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bj::string_list values = ::eval_eq_expr(ctx, *(beg++));
    while (values && (beg != end))
    {
        ++beg;
        values = ::eval_eq_expr(ctx, *(beg++));
    }

    return values;
}

bj::string_list eval_expr(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bj::string_list values = ::eval_and_expr(ctx, *(beg++));
    while (!values && (beg != end))
    {
        ++beg;
        values = ::eval_and_expr(ctx, *(beg++));
    }

    return values;
}

bj::string_list eval_invoke_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::invoke_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    const std::string& name = ::node_value(*(beg++));
    bj::list_of_list args;
    if (beg->value.id() == bj::lol_id)
        args = ::eval_lol(ctx, *beg);
    return ctx.invoke_rule(name, args);
}

bj::assign_mode::values eval_assign(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::assign_id);
    assert((tree.children.size() == 1) || (tree.children.size() == 2));

    switch (*tree.children[0].value.begin())
    {
        default:
            assert(0);
        case '=':
            return bj::assign_mode::set_default;
        case '+':
            return bj::assign_mode::append;
        case '?':
        case 'd':
            return bj::assign_mode::set_default;
    }
}

bj::string_list eval_set_on_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::set_on_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    const bj::string_list& names = ::eval_arg(ctx, *(beg++));
    ++beg;
    bj::string_list targets;
    if (beg->value.id() == bj::list_id)
        targets = ::eval_list(ctx, *(beg++));
    bj::assign_mode::values mode = ::eval_assign(ctx, *(beg++));
    bj::string_list values;
    if (beg->value.id() == bj::list_id)
        values = ::eval_list(ctx, *(beg++));

    bj::frame& f = ctx.current_frame();
    bj::module& m = f.current_module();

    for (std::size_t i = 0; i < targets.size(); ++i)
    {
        bj::variable_table& table = ctx.get_target(targets[i]).variables;
        bj::set_variables(table, mode, names, values);
    }

    return values;
}

bj::string_list eval_set_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::set_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    const bj::string_list& names = ::eval_arg(ctx, *(beg++));
    bj::assign_mode::values mode = ::eval_assign(ctx, *(beg++));
    bj::string_list values;
    if (beg->value.id() == bj::list_id)
        values = ::eval_list(ctx, *(beg++));

    bj::frame& f = ctx.current_frame();
    bj::module& m = f.current_module();

    bj::set_variables(m.variables, mode, names, values);
    return values;
}

void eval_for_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::for_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bool local = false;
    if (::node_value(*++beg) == "local")
    {
        local = true;
        ++beg;
    }

    const std::string& var = ::node_value(*(beg++));
    ++beg;
    bj::string_list values;
    if (beg->value.id() == bj::list_id)
        values = ::eval_list(ctx, *(beg++));
    ++beg;

    bj::frame& f = ctx.current_frame();
    bj::variable_table& table = f.current_module().variables;
    bj::scoped_swap_values guard(table, var, local);
    for (std::size_t i = 0; i < values.size(); ++i)
    {
        table.set_values(var, bj::string_list(values[i]));
        if (beg->value.id() == bj::block_id)
            ::eval_block(ctx, *beg);
    }
}

bj::string_list
eval_cases(bj::context& ctx, const node_t& tree, const std::string& value)
{
    assert(tree.value.id() == bj::cases_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    while (beg != end)
    {
        ++beg;
        const std::string& pattern = ::node_value(*(beg++));

        if (++beg == end)
            break;

        if (beg->value.id() != bj::block_id)
            continue;

        if (bj::pattern_match(pattern, value))
            return ::eval_block(ctx, *beg);
        else
            ++beg;
    }
    return bj::string_list();
}

bj::string_list eval_switch_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::switch_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    ++beg;
    std::string value;
    if (beg->value.id() == bj::list_id)
    {
        const bj::string_list& list = ::eval_list(ctx, *beg);
        if (!list.empty())
            value = list[0];
        ++beg;
    }

    ++beg;
    if (beg->value.id() == bj::cases_id)
        return ::eval_cases(ctx, *beg, value);
    else
        return bj::string_list();
}

bj::string_list eval_module_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::module_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    ++beg;
    bj::string_list list;
    if (beg->value.id() == bj::list_id)
        list = ::eval_list(ctx, *(beg++));

    bj::scoped_change_module guard(ctx, list.try_front());

    ++beg;
    if (beg->value.id() == bj::block_id)
        return ::eval_block(ctx, *beg);
    else
        return bj::string_list();
}

void eval_class_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::class_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    ++beg;
    if (beg->value.id() != bj::lol_id)
        throw std::runtime_error("missing class name"); // FIXME

    const bj::list_of_list& lol = ::eval_lol(ctx, *(beg++));

    if (lol[0].empty())
        throw std::runtime_error("missing class name"); // FIXME

    const std::string& module_name = bj::make_class(ctx, lol[0][0], lol[1]);

    bj::scoped_change_module guard(ctx, module_name);
    ++beg;
    if (beg->value.id() == bj::block_id)
        ::eval_block(ctx, *beg);
}

bj::string_list eval_while_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::while_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    ++beg;
    const node_t& cond = *(beg++);
    ++beg;
    bool has_block = (beg->value.id() == bj::block_id);

    bj::string_list values;
    while (::eval_expr(ctx, cond))
    {
        if (has_block)
            values = ::eval_block(ctx, *beg);
    }
    return values;
}

bj::list_of_list eval_arglist(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::arglist_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    ++beg;
    if (beg->value.id() == bj::lol_id)
        return ::eval_lol(ctx, *beg);
    else
        return bj::list_of_list();
}

void eval_rule_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::rule_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bj::rule_definition def;
    if (::node_value(*++beg) == "local")
    {
        def.exported = false;
        ++beg;
    }

    const std::string& name = ::node_value(*(beg++));
    if (beg->value.id() == bj::arglist_id)
        def.parameters = ::eval_arglist(ctx, *(beg++));
    def.body = boost::bind(&eval_rule, _1, boost::cref(*beg));

    bj::frame& f = ctx.current_frame();
    bj::rule_table& table = f.current_module().rules;
    table.set_rule_definition(name, def);
}

bj::string_list eval_rule(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::rule_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    if (beg->value.id() == bj::invoke_stmt_id)
        return ::eval_invoke_stmt(ctx, *beg);
    else if (beg->value.id() == bj::set_stmt_id)
        return ::eval_set_stmt(ctx, *beg);
    else if (beg->value.id() == bj::set_on_stmt_id)
        return ::eval_set_on_stmt(ctx, *beg);
    else if (beg->value.id() == bj::for_stmt_id)
    {
        ::eval_for_stmt(ctx, *beg);
        return bj::string_list();
    }
    else if (beg->value.id() == bj::switch_stmt_id)
        return ::eval_switch_stmt(ctx, *beg);
    else if (beg->value.id() == bj::module_stmt_id)
        return ::eval_module_stmt(ctx, *beg);
    else if (beg->value.id() == bj::class_stmt_id)
    {
        ::eval_class_stmt(ctx, *beg);
        return bj::string_list();
    }
    else if (beg->value.id() == bj::while_stmt_id)
        return ::eval_while_stmt(ctx, *beg);
    else if (beg->value.id() == bj::rule_stmt_id)
    {
        eval_rule_stmt(ctx, *beg);
        return bj::string_list();
    }
    else
    {
        const std::string& x = ::node_value(*(beg++));
        if (x == "return")
        {
            if (beg->value.id() == bj::list_id)
                return ::eval_list(ctx, *beg);
            else
                return bj::string_list();
        }
        else if (x == "{")
        {
            if (beg->value.id() == bj::block_id)
                return ::eval_block(ctx, *beg);
            else
                return bj::string_list();
        }

        // TODO
        return bj::string_list();
    }
}

bj::string_list eval_assign_list(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::assign_list_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    ++beg;
    if (beg->value.id() == bj::list_id)
        return ::eval_list(ctx, *beg);
    else
        return bj::string_list();
}

bj::string_list eval_local_set_stmt(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::set_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    ++beg;
    bj::string_list names;
    if (beg->value.id() == bj::list_id)
        names = ::eval_list(ctx, *(beg++));

    bj::string_list values;
    if (beg->value.id() == bj::assign_list_id)
        values = ::eval_assign_list(ctx, *(beg++));

    bj::variable_table local;
    for (std::size_t i = 0; i < names.size(); ++i)
        local.set_values(names[i], values);

    bj::frame& f = ctx.current_frame();
    bj::module& m = f.current_module();
    bj::scoped_push_local_variables using_local(m.variables, local);

    if (beg->value.id() == bj::block_id)
        return ::eval_block(ctx, *beg);
    else
        return bj::string_list();
}

bj::string_list eval_rules(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::rules_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    if (beg->value.id() == bs::parser_id(bj::rule_id))
    {
        bj::string_list values = ::eval_rule(ctx, *beg);
        while (++beg != end)
            values = ::eval_rules(ctx, *beg);
        return values;
    }
    else
    {
        // TODO
        return bj::string_list();
    }
}

bj::string_list eval_block(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::block_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if (beg != end)
        return ::eval_rules(ctx, *beg);
    else
        return bj::string_list();
}

bj::string_list eval_run(bj::context& ctx, const node_t& tree)
{
    assert(tree.value.id() == bj::run_id);

    if (!tree.children.empty())
        return ::eval_rules(ctx, tree.children.front());
    else
        return bj::string_list();
}

bj::string_list evaluate(bj::context& ctx, const bs::tree_parse_info<>& info)
{
    return ::eval_run(ctx, info.trees[0]);
}


#include <fstream>
#include <sstream>

bj::string_list echo(bj::context& ctx)
{
    const bj::list_of_list& args = ctx.current_frame().arguments();

    std::cout << args[0] << std::endl;
    return bj::string_list();
}

bs::tree_parse_info<> parse_bjam(const char* s)
{
    return bj::bjam_grammar_gen<const char*>::
        parse_bjam_grammar(s, s+std::strlen(s));
}

bs::tree_parse_info<> parse_bjam(const char* s, std::size_t n)
{
    return bj::bjam_grammar_gen<const char*>::parse_bjam_grammar(s, s+n);
}

int main()
{
    try
    {
        std::ifstream is("test.jam");
        std::string src(
            std::istreambuf_iterator<char>(is),
            (std::istreambuf_iterator<char>())
        );
        is.close();

        bs::tree_parse_info<> info = ::parse_bjam(src.c_str(), src.size());

        if (info.full)
        {
            bj::context ctx;

            // setup the built-in rules
            bj::list_of_list params;
            ctx.set_builtin_rule("ECHO", params, &echo);

            std::cout << ::evaluate(ctx, info) << std::endl;
        }
        else
            std::cerr << "failed" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
