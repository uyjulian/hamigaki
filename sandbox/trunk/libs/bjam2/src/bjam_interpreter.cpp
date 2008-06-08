// bjam_interpreter.cpp: bjam interpreter

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM2_SOURCE
#include <hamigaki/bjam2/bjam_interpreter.hpp>
#include <hamigaki/bjam2/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam2/grammars/bjam_grammar_id.hpp>
#include <hamigaki/bjam2/util/class.hpp>
#include <hamigaki/bjam2/util/list_of_list.hpp>
#include <hamigaki/bjam2/util/pattern.hpp>
#include <hamigaki/bjam2/util/search.hpp>
#include <hamigaki/bjam2/util/variable_expansion.hpp>
#include <hamigaki/bjam2/bjam_context.hpp>
#include <hamigaki/bjam2/bjam_exceptions.hpp>
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <boost/ref.hpp>
#include <cassert>
#include <fstream>
#include <iterator>

namespace hamigaki { namespace bjam2 {

namespace
{

typedef tree_parse_info<> parse_info_t;
typedef tree_node::const_tree_iterator iter_t;

const string_list true_value("1");

parse_info_t parse_bjam(const char* s, std::size_t n)
{
    return bjam2::bjam_grammar_gen<const char*>::parse_bjam_grammar(s, s+n);
}

std::string node_value(const tree_node& node)
{
    return std::string(node.value.begin(), node.value.end());
}

std::string eval_arg_p(const tree_node& node)
{
    return std::string(node.value.begin(), node.value.end());
}

std::string eval_non_punct_p(const tree_node& node)
{
    return std::string(node.value.begin(), node.value.end());
}

void set_true(string_list& lhs, const string_list& rhs)
{
    if (lhs)
        return;

    if (rhs)
        lhs = rhs;
    else
        lhs = true_value;
}

list_of_list
concatenate_args(const string_list& args0, const list_of_list& args)
{
    if (args0.size() == 1)
        return args;

    string_list arg(boost::next(args0.begin()), args0.end());

    list_of_list tmp;
    arg += args[0];
    tmp.push_back(arg);
    for (std::size_t i = 1, size = args.size(); i < size; ++i)
        tmp.push_back(args[i]);
    return tmp;
}

bool includes(const string_list& lhs, const string_list& rhs)
{
    typedef string_list::const_iterator iter_type;

    iter_type lb = lhs.begin();
    iter_type le = lhs.end();
    iter_type rb = rhs.begin();
    iter_type re = rhs.end();

    for (iter_type i = lb; i != le; ++i)
        if (std::find(rb, re, *i) == re)
            return false;

    return true;
}

} // namespace

string_list eval_arg(context& ctx, const tree_node& tree);
string_list eval_list(context& ctx, const tree_node& tree);
list_of_list eval_lol(context& ctx, const tree_node& tree);
string_list eval_expr(context& ctx, const tree_node& tree);
string_list eval_rule(context& ctx, const tree_node& tree);
string_list eval_block(context& ctx, const tree_node& tree);
string_list eval_run(context& ctx, const tree_node& tree);

string_list eval_func_impl(context& ctx, iter_t beg, iter_t end)
{
    int line = beg->children.front().value.line();

    const string_list& args0 = bjam2::eval_arg(ctx, *(beg++));
    list_of_list args;
    if (beg != end)
        args = bjam2::eval_lol(ctx, *beg);

    if (!args0.empty())
    {
        frame& f = ctx.current_frame();
        f.line(line);

        return ctx.invoke_rule(args0[0], concatenate_args(args0, args));
    }
    else
        return string_list();
}

string_list eval_func(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::func_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    if (beg->value.id() == bjam2::arg_id)
        return bjam2::eval_func_impl(ctx, beg, end);
    else
    {
        ++beg;
        const string_list& targets = bjam2::eval_arg(ctx, *(beg++));
        if (targets.empty())
            return string_list();

        scoped_on_target guard(ctx, targets[0]);
        if (beg->value.id() == bjam2::arg_id)
            return bjam2::eval_func_impl(ctx, beg, end);
        else
        {
            ++beg;
            if (beg->value.id() == bjam2::list_id)
                return bjam2::eval_list(ctx, *beg);
            else
                return string_list();
        }
    }
}

string_list eval_non_punct(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::non_punct_id);
    assert(!tree.children.empty());

    if (tree.children.front().value.id() != bjam2::func_id)
    {
        frame& f = ctx.current_frame();
        const variable_table& table = f.current_module().variables;
        const list_of_list& args = f.arguments();

        const std::string& name =
            bjam2::eval_non_punct_p(tree.children.front());
        return bjam2::expand_variable(name, table, args);
    }
    else
        return bjam2::eval_func(ctx, tree.children.front());
}

string_list eval_arg(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::arg_id);
    assert(!tree.children.empty());

    if (tree.children.front().value.id() != bjam2::func_id)
    {
        frame& f = ctx.current_frame();
        const variable_table& table = f.current_module().variables;
        const list_of_list& args = f.arguments();

        const std::string& name = bjam2::eval_arg_p(tree.children.front());
        return bjam2::expand_variable(name, table, args);
    }
    else
        return bjam2::eval_func(ctx, tree.children.front());
}

string_list eval_list(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::list_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    string_list list;
    for ( ; beg != end; ++beg)
    {
        if (beg->value.id() == bjam2::non_punct_id)
            list += bjam2::eval_non_punct(ctx, *beg);
    }
    return list;
}

list_of_list eval_lol(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::lol_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    list_of_list values;
    bool colon = true;
    for ( ; beg != end; ++beg)
    {
        if (beg->value.id() == bjam2::list_id)
        {
            values.push_back(bjam2::eval_list(ctx, *beg));
            colon = false;
        }
        else if (colon)
            values.push_back(string_list());
        else
            colon = true;
    }

    if (colon)
        values.push_back(string_list());

    return values;
}

string_list eval_prim_expr(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::prim_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    if (beg->value.id() == bjam2::arg_id)
    {
        string_list values = bjam2::eval_arg(ctx, *(beg++));
        if (beg != end)
        {
            if (values.empty())
                return true_value;
            else if (beg == end)
                return string_list();

            const string_list& rhs = bjam2::eval_list(ctx, *(beg++));
            if (includes(values, rhs))
                set_true(values, rhs);
            else
                values.clear();
        }
        return values;
    }
    else
        return bjam2::eval_expr(ctx, tree.children.front());
}

string_list eval_not_expr(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::not_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    bool not_op = false;
    while (beg->value.id() != bjam2::prim_expr_id)
    {
        not_op = !not_op;
        ++beg;
    }

    const string_list& values = bjam2::eval_prim_expr(ctx, *(beg++));

    if (not_op)
    {
        if (values)
            return string_list();
        else
            return true_value;
    }
    else
        return values;
}

string_list eval_rel_expr(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::rel_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    string_list values = bjam2::eval_not_expr(ctx, *(beg++));
    while (beg != end)
    {
        tree_node::parse_node_t op = (beg++)->value;
        const string_list& rhs = bjam2::eval_not_expr(ctx, *(beg++));

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
            set_true(values, rhs);
        else
            values.clear();
    }

    return values;
}

string_list eval_eq_expr(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::eq_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    string_list values = bjam2::eval_rel_expr(ctx, *(beg++));
    while (beg != end)
    {
        bool op = (*(beg++)->value.begin() == '=');
        const string_list& rhs = bjam2::eval_rel_expr(ctx, *(beg++));
        if ((values == rhs) == op)
            set_true(values, rhs);
        else
            values.clear();
    }

    return values;
}

string_list eval_and_expr(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::and_expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    string_list values = bjam2::eval_eq_expr(ctx, *(beg++));
    if (!values)
        return values;

    while (beg != end)
    {
        const string_list& rhs = bjam2::eval_eq_expr(ctx, *(beg++));
        if (!rhs)
            return rhs;
    }

    return values;
}

string_list eval_expr(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::expr_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    string_list values = bjam2::eval_and_expr(ctx, *(beg++));
    while (!values && (beg != end))
        values = bjam2::eval_and_expr(ctx, *(beg++));

    return values;
}

string_list eval_block_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::block_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if (beg != end)
        return bjam2::eval_block(ctx, *beg);
    else
        return string_list();
}

string_list eval_include_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::include_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if (beg == end)
        return string_list();

    const string_list& names = bjam2::eval_list(ctx, *beg);
    if (names.empty())
        return string_list();

    const std::string& target_name = names[0];
    scoped_on_target gurad(ctx, target_name);
    const std::string& filename = bjam2::search_target(ctx, target_name);

    std::ifstream is(filename.c_str(), std::ios_base::binary);
    if (!is)
        throw cannot_open_file(filename);

    std::string str(
        std::istreambuf_iterator<char>(is),
        (std::istreambuf_iterator<char>())
    );
    is.close();

    scoped_change_filename guard(ctx.current_frame(), filename);
    parse_info_t info = parse_bjam(str.c_str(), str.size());

    bjam2::eval_run(ctx, ctx.push_parse_tree(info.trees.front()));

    return string_list();
}

string_list eval_invoke_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::invoke_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    const std::string& name = bjam2::eval_arg_p(*(beg++));

    frame& f = ctx.current_frame();
    const variable_table& table = f.current_module().variables;
    const string_list& args0 =
        bjam2::expand_variable(name, table, f.arguments());

    list_of_list args;
    if (beg != end)
        args = bjam2::eval_lol(ctx, *beg);

    if (args0.empty())
        return string_list();

    f.line(tree.children.front().value.line());

    return ctx.invoke_rule(args0[0], concatenate_args(args0, args));
}

assign_mode::values eval_assign(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::assign_id);
    assert((tree.children.size() == 1) || (tree.children.size() == 2));

    switch (*tree.children[0].value.begin())
    {
        default:
            assert(0);
        case '=':
            return assign_mode::set;
        case '+':
            return assign_mode::append;
        case '?':
        case 'd':
            return assign_mode::set_default;
    }
}

string_list eval_set_on_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::set_on_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    const string_list& names = bjam2::eval_arg(ctx, *(beg++));
    string_list targets;
    if (beg->value.id() == bjam2::list_id)
        targets = bjam2::eval_list(ctx, *(beg++));
    assign_mode::values mode = bjam2::eval_assign(ctx, *(beg++));
    string_list values;
    if (beg != end)
        values = bjam2::eval_list(ctx, *beg);

    for (std::size_t i = 0, size = targets.size(); i < size; ++i)
    {
        variable_table& table = ctx.get_target(targets[i]).variables;
        bjam2::set_variables(table, mode, names, values);
    }

    return values;
}

string_list eval_set_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::set_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    const string_list& names = bjam2::eval_arg(ctx, *(beg++));
    assign_mode::values mode = bjam2::eval_assign(ctx, *(beg++));
    string_list values;
    if (beg != end)
        values = bjam2::eval_list(ctx, *beg);

    frame& f = ctx.current_frame();
    module& m = f.current_module();

    bjam2::set_variables(m.variables, mode, names, values);
    return values;
}

string_list eval_return_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::return_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if (beg != end)
        return bjam2::eval_list(ctx, *beg);
    else
        return string_list();
}

inline bool is_for_local(const tree_node::children_t& children)
{
    typedef tree_node::children_t::const_reverse_iterator iter_t;

    iter_t beg = children.rbegin();
    iter_t end = children.rend();

    if (beg->value.id() == bjam2::block_id)
        ++beg;
    if (beg->value.id() == bjam2::list_id)
        ++beg;

    return ++beg != end;
}

string_list eval_for_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::for_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    bool local = is_for_local(tree.children);
    if (local)
        ++beg;

    const std::string& var = bjam2::eval_arg_p(*(beg++));
    string_list values;
    if ((beg != end) && (beg->value.id() == bjam2::list_id))
        values = bjam2::eval_list(ctx, *(beg++));

    frame& f = ctx.current_frame();
    variable_table& table = f.current_module().variables;
    scoped_swap_values guard(table, var, local);
    for (std::size_t i = 0, size = values.size(); i < size; ++i)
    {
        table.set_values(var, string_list(values[i]));
        if (beg != end)
            bjam2::eval_block(ctx, *beg);
    }

    return string_list();
}

string_list
eval_cases(context& ctx, const tree_node& tree, const std::string& value)
{
    assert(tree.value.id() == bjam2::cases_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    while (beg != end)
    {
        const std::string& pattern = bjam2::eval_arg_p(*(beg++));

        if (beg == end)
            break;

        if (bjam2::pattern_match(pattern, value))
        {
            if (beg->value.id() == bjam2::block_id)
                return bjam2::eval_block(ctx, *beg);
            else
                return string_list();
        }
        else
        {
            if (beg->value.id() == bjam2::block_id)
                ++beg;
        }
    }
    return string_list();
}

string_list eval_switch_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::switch_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    std::string value;
    if ((beg != end) && (beg->value.id() == bjam2::list_id))
    {
        const string_list& list = bjam2::eval_list(ctx, *beg);
        if (!list.empty())
            value = list[0];
        ++beg;
    }

    if (beg != end)
        return bjam2::eval_cases(ctx, *beg, value);
    else
        return string_list();
}

string_list eval_module_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::module_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    string_list list;
    if ((beg != end) && (beg->value.id() == bjam2::list_id))
        list = bjam2::eval_list(ctx, *(beg++));

    scoped_change_module guard(ctx, list.try_front());

    if (beg != end)
        return bjam2::eval_block(ctx, *beg);
    else
        return string_list();
}

string_list eval_class_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::class_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if ((beg != end) && (beg->value.id() != bjam2::lol_id))
        throw std::runtime_error("missing class name"); // FIXME

    const list_of_list& lol = bjam2::eval_lol(ctx, *(beg++));

    if (lol[0].empty())
        throw std::runtime_error("missing class name"); // FIXME

    const std::string& module_name = bjam2::make_class(ctx, lol[0][0], lol[1]);

    scoped_change_module guard(ctx, module_name);
    if (beg != end)
        bjam2::eval_block(ctx, *beg);

    return string_list();
}

string_list eval_while_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::while_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    const tree_node& cond = *(beg++);
    bool has_block = (beg != end);

    string_list values;
    while (bjam2::eval_expr(ctx, cond))
    {
        if (has_block)
            values = bjam2::eval_block(ctx, *beg);
    }
    return values;
}

string_list eval_if_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::if_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    const string_list& cond = bjam2::eval_expr(ctx, *(beg++));

    string_list values;
    if (cond)
    {
        if ((beg != end) && (beg->value.id() == bjam2::block_id))
            values = bjam2::eval_block(ctx, *beg);
    }
    else
    {
        if ((beg != end) && (beg->value.id() == bjam2::block_id))
            ++beg;

        if (beg != end)
            values = bjam2::eval_rule(ctx, *beg);
    }
    return values;
}

list_of_list eval_arglist(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::arglist_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if (beg != end)
        return bjam2::eval_lol(ctx, *beg);
    else
        return list_of_list();
}

inline bool is_local_rule(const tree_node::children_t& children)
{
    typedef tree_node::children_t::const_reverse_iterator iter_t;

    iter_t beg = children.rbegin();
    iter_t end = children.rend();

    if (beg->value.id() == bjam2::rule_id)
        ++beg;
    if (beg->value.id() == bjam2::arglist_id)
        ++beg;

    return ++beg != end;
}

string_list eval_rule_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::rule_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    rule_definition def;
    if (is_local_rule(tree.children))
    {
        def.exported = false;
        ++beg;
    }

    const std::string& name = bjam2::eval_arg_p(*(beg++));
    if ((beg != end) && (beg->value.id() == bjam2::arglist_id))
        def.parameters = bjam2::eval_arglist(ctx, *(beg++));
    def.body = boost::bind(&eval_rule, _1, boost::cref(*beg));

    frame& f = ctx.current_frame();
    def.module_name = f.module_name();
    def.filename = f.filename();
    def.line = tree.children.front().value.line();

    rule_table& table = f.current_module().rules;
    table.set_rule_body(name, def);

    return string_list();
}

string_list eval_on_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::on_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    const string_list& targets = bjam2::eval_arg(ctx, *(beg++));
    if (targets.empty())
        return string_list();

    scoped_on_target guard(ctx, targets[0]);
    return bjam2::eval_rule(ctx, *beg);
}

action_modifier::values eval_eflags(const tree_node& tree)
{
    assert(tree.value.id() == bjam2::eflags_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    action_modifier::values eflags = action_modifier::values();
    for ( ; beg != end; ++beg)
    {
        switch (*beg->value.begin())
        {
            default:
                assert(0);
            case 'u':
                eflags = eflags | action_modifier::updated;
                break;
            case 't':
                eflags = eflags | action_modifier::together;
                break;
            case 'i':
                eflags = eflags | action_modifier::ignore;
                break;
            case 'q':
                eflags = eflags | action_modifier::quietly;
                break;
            case 'p':
                eflags = eflags | action_modifier::piecemeal;
                break;
            case 'e':
                eflags = eflags | action_modifier::existing;
                break;
        }
    }
    return eflags;
}

string_list eval_bindlist(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::bindlist_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if (beg == end)
        return bjam2::eval_list(ctx, *beg);
    else
        return string_list();
}

string_list eval_actions_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::actions_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    rule_definition def;
    if (beg->value.id() == bjam2::eflags_id)
        def.modifiers = bjam2::eval_eflags(*(beg++));

    const std::string& name = bjam2::eval_arg_p(*(beg++));

    if (beg->value.id() == bjam2::bindlist_id)
        def.binds = bjam2::eval_bindlist(ctx, *(beg++));

    def.commands = node_value(*beg);

    frame& f = ctx.current_frame();
    rule_table& table = f.current_module().rules;

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

    return string_list();
}

string_list eval_rule(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::rule_id);
    assert(!tree.children.empty());

    const tree_node& child = tree.children.front();
    switch (child.value.id().to_long())
    {
        default:
            assert(0);
        case bjam2::block_stmt_id:
            return bjam2::eval_block_stmt(ctx, child);
        case bjam2::include_stmt_id:
            return bjam2::eval_include_stmt(ctx, child);
        case bjam2::invoke_stmt_id:
            return bjam2::eval_invoke_stmt(ctx, child);
        case bjam2::set_stmt_id:
            return bjam2::eval_set_stmt(ctx, child);
        case bjam2::set_on_stmt_id:
            return bjam2::eval_set_on_stmt(ctx, child);
        case bjam2::return_stmt_id:
            return bjam2::eval_return_stmt(ctx, child);
        case bjam2::for_stmt_id:
            return bjam2::eval_for_stmt(ctx, child);
        case bjam2::switch_stmt_id:
            return bjam2::eval_switch_stmt(ctx, child);
        case bjam2::module_stmt_id:
            return bjam2::eval_module_stmt(ctx, child);
        case bjam2::class_stmt_id:
            return bjam2::eval_class_stmt(ctx, child);
        case bjam2::while_stmt_id:
            return bjam2::eval_while_stmt(ctx, child);
        case bjam2::if_stmt_id:
            return bjam2::eval_if_stmt(ctx, child);
        case bjam2::rule_stmt_id:
            return bjam2::eval_rule_stmt(ctx, child);
        case bjam2::on_stmt_id:
            return bjam2::eval_on_stmt(ctx, child);
        case bjam2::actions_stmt_id:
            return bjam2::eval_actions_stmt(ctx, child);
    }
}

string_list eval_assign_list(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::assign_list_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if (beg != end)
        return bjam2::eval_list(ctx, *beg);
    else
        return string_list();
}

string_list eval_local_set_stmt(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::local_set_stmt_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    string_list names;
    if ((beg != end) && (beg->value.id() == bjam2::list_id))
        names = bjam2::eval_list(ctx, *(beg++));

    string_list values;
    if ((beg != end) && (beg->value.id() == bjam2::assign_list_id))
        values = bjam2::eval_assign_list(ctx, *(beg++));

    variable_table local;
    for (std::size_t i = 0, size = names.size(); i < size; ++i)
        local.set_values(names[i], values);

    frame& f = ctx.current_frame();
    module& m = f.current_module();
    scoped_push_local_variables using_local(m.variables, local);

    if (beg != end)
        return bjam2::eval_block(ctx, *beg);
    else
        return string_list();
}

string_list eval_rules(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::rules_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();
    assert(beg != end);

    if (beg->value.id() == bjam2::rule_id)
    {
        string_list values = bjam2::eval_rule(ctx, *(beg++));
        if (beg != end)
            values = bjam2::eval_rules(ctx, *beg);
        return values;
    }
    else
        return bjam2::eval_local_set_stmt(ctx, *beg);
}

string_list eval_block(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::block_id);

    iter_t beg = tree.children.begin();
    iter_t end = tree.children.end();

    if (beg != end)
        return bjam2::eval_rules(ctx, *beg);
    else
        return string_list();
}

string_list eval_run(context& ctx, const tree_node& tree)
{
    assert(tree.value.id() == bjam2::run_id);

    if (!tree.children.empty())
    {
        const tree_node& child = tree.children.front();

        if (child.value.id() == bjam2::rules_id)
            return bjam2::eval_rules(ctx, child);
        else
            return string_list();
    }
    else
        return string_list();
}

HAMIGAKI_BJAM2_DECL
string_list evaluate_expression(context& ctx, const tree_node& tree)
{
    return bjam2::eval_expr(ctx, tree);
}

HAMIGAKI_BJAM2_DECL
string_list evaluate_bjam(context& ctx, const tree_node& tree)
{
    return bjam2::eval_run(ctx, tree);
}

} } // End namespaces bjam2, hamigaki.
