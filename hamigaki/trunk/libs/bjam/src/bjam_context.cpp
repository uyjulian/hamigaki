// bjam_context.cpp: the context information for bjam

// Copyright Takeshi Mouri 2007-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/bjam_context.hpp>
#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>
#include <locale>
#include <sstream>

namespace fs = boost::filesystem;

namespace hamigaki { namespace bjam {

namespace
{

const rule_definition* get_imported_rule_ptr_impl(
    const context& ctx, const module& m, 
    const std::string& module_name, const std::string& rule_name,
    bool& is_instance)
{
    std::string class_name;

    const std::set<std::string>& table = m.imported_modules;
    if (table.find(module_name) == table.end())
        return 0;

    const module* mp = &ctx.get_module(module_name);
    if (mp->class_module)
    {
        class_name = *mp->class_module;
        mp = &ctx.get_module(class_name);
    }

    const rule_definition* ptr = mp->rules.get_rule_definition_ptr(rule_name);
    if (ptr && ptr->exported)
    {
        if (!class_name.empty() &&
            ptr->module_name && (*ptr->module_name == class_name) )
        {
            is_instance = true;
        }
        return ptr;
    }
    else
        return 0;
}

const rule_definition*
get_rule_definition_ptr_impl(
    const context& ctx, const module& m, const std::string& name,
    boost::optional<std::string>& rule_module_name)
{
    typedef const rule_definition* ptr_type;

    std::string class_name;
    const module* mp = &m;
    if (mp->class_module)
    {
        class_name = *mp->class_module;
        mp = &ctx.get_module(class_name);
    }

    if (ptr_type p = mp->rules.get_rule_definition_ptr(name))
    {
        if (class_name.empty() ||
            !p->module_name ||
            (*p->module_name != class_name) )
        {
            rule_module_name = p->module_name;
        }
        return p;
    }

    std::string::size_type dot = name.find('.');
    if (dot == std::string::npos)
        return 0;

    std::string module_name(name, 0u, dot);
    std::string rule_name(name, dot+1);

    bool is_instance = false;
    ptr_type p = get_imported_rule_ptr_impl(
        ctx, *mp, module_name, rule_name, is_instance);
    if (p)
    {
        if (is_instance)
            rule_module_name = module_name;
        else
            rule_module_name = p->module_name;
    }
    return p;
}

void set_rule_argument(
    variable_table& table, const string_list& param, const string_list& arg)
{
    typedef string_list::const_iterator iter_type;

    iter_type p = param.begin();
    iter_type a = arg.begin();

    while (p != param.end())
    {
        std::string type_check;
        if ((p->size() >= 2) && (*p->begin() == '[') && (*p->rbegin() == ']'))
            type_check = *(p++);

        std::string name;
        if ((*p != "?") && (*p != "*") && (*p != "+"))
            name = *(p++);

        std::string opt;
        if (p != param.end())
        {
            if ((*p == "?") || (*p == "*") || (*p == "+"))
                opt = *(p++);
        }

        string_list values;
        if (opt == "?")
        {
            if (a != arg.end())
                values.push_back(*(a++));
        }
        else if (opt == "*")
        {
            values = string_list(a, arg.end());
            a = arg.end();
        }
        else
        {
            if (a == arg.end())
                throw std::runtime_error("missing a rule argument"); // TODO

            if (opt == "+")
            {
                values = string_list(a, arg.end());
                a = arg.end();
            }
            else
                values.push_back(*(a++));
        }

        if (!name.empty())
            table.set_values(name, values);
    }
}

} // namespace

context::context()
    : working_directory_(fs::current_path<fs::path>().directory_string())
    , os_(&std::cout)
{
    frames_.push_back(frame(root_module_));
    set_predefined_variables(*this);
    set_builtin_rules(*this);
    set_native_rules(*this);
}

boost::optional<std::string>
context::caller_module_name(std::size_t level) const
{
    frame_stack::const_reverse_iterator pos = frames_.rbegin();
    frame_stack::const_reverse_iterator end = frames_.rend();
    while (pos != end)
    {
        if (level == 0)
            return pos->module_name();

        ++pos;
        --level;
    }
    return boost::optional<std::string>();
}

string_list context::back_trace(std::size_t level, std::size_t skip) const
{
    string_list result;
    frame_stack::const_reverse_iterator pos = frames_.rbegin();
    frame_stack::const_reverse_iterator end = frames_.rend();

    for (std::size_t i = 0; i < skip; ++i)
    {
        if (pos == end)
            return result;
        ++pos;
    }

    for (std::size_t i = 0; i < level; ++i)
    {
        if (pos == end)
            break;

        const frame& f = *pos;

        result.push_back(f.filename());

        {
            std::ostringstream os;
            os.imbue(std::locale::classic());
            os << f.line();
            result.push_back(os.str());
        }

        if (f.module_name())
            result.push_back(*f.module_name() + ".");
        else
            result.push_back(std::string());

        if (f.rule_name())
            result.push_back(*f.rule_name());
        else
            result.push_back("module scope");

        ++pos;
    }
    return result;
}

module& context::get_module(const boost::optional<std::string>& name)
{
    if (name)
        return modules_[*name];
    else
        return root_module_;
}

const module&
context::get_module(const boost::optional<std::string>& name) const
{
    if (name)
    {
        typedef std::map<std::string,module>::const_iterator iter_type;
        iter_type i = modules_.find(*name);
        if (i == modules_.end())
            throw module_not_found(*name);
        return i->second;
    }
    else
        return root_module_;
}

void context::change_module(const boost::optional<std::string>& name)
{
    frame& f = current_frame();

    if (f.module_name() == name)
        return;

    if (name)
        f.change_module(modules_[*name], name);
    else
        f.change_module(root_module_, name);
}

void context::set_builtin_rule(
    const std::string& name, const list_of_list& params,
    const boost::function1<string_list,context&>& func,
    bool exported)
{
    rule_definition def;

    def.parameters = params;
    def.native = func;
    def.exported = exported;

    root_module_.rules.set_native_rule(name, def);
}

rule_definition context::get_rule_definition(const std::string& name) const
{
    typedef const rule_definition* ptr_type;

    const frame& f = frames_.back();
    const module& m = f.current_module();

    boost::optional<std::string> module_name = f.module_name();
    ptr_type p = get_rule_definition_ptr_impl(*this, m, name, module_name);
    if (p)
    {
        rule_definition def(*p);
        def.module_name = module_name;
        return def;
    }

    module_name = boost::optional<std::string>();
    p = get_rule_definition_ptr_impl(*this, root_module_, name, module_name);
    if (!p)
        throw rule_not_found(name);

    rule_definition def(*p);
    def.module_name = module_name;
    return def;
}

string_list
context::invoke_rule(const std::string& name, const list_of_list& args)
{
    const rule_definition& rule = this->get_rule_definition(name);

    frame& old = current_frame();

    frame f(old.current_module(), old.module_name());
    f.rule_name(name);
    f.arguments() = args;
    f.filename(rule.filename);
    f.line(rule.line);
    f.prev_user_frame(old.prev_user_frame());

    scoped_push_frame guard(*this, f);
    this->change_module(rule.module_name);

    frame& cur_frame = current_frame();
    module& cur_module = cur_frame.current_module();

    variable_table local;
    const list_of_list& params = rule.parameters;
    for (std::size_t i = 0; i < params.size(); ++i)
        set_rule_argument(local, params[i], args[i]);

    if (rule.native)
        return rule.native(*this);
    else if (rule.body.get() != 0)
    {
        scoped_push_local_variables using_local(cur_module.variables, local);

        // Note: make a copy for the re-definition of the rule
        boost::shared_ptr<std::string> body = rule.body;
        const char* first = body->c_str();
        const char* last = first + body->size();

        typedef bjam::bjam_grammar_gen<const char*> grammar_type;

        return
            grammar_type::parse_bjam_grammar(
                first, last, *this, rule.line
            ).values;
    }
    else
        return string_list();
}

} } // End namespaces bjam, hamigaki.
