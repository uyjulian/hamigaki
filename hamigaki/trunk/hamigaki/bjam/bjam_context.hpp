// bjam_context.hpp: the context information for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_BJAM_CONTEXT_HPP
#define HAMIGAKI_BJAM_BJAM_CONTEXT_HPP

#include <hamigaki/bjam/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam/util/frame.hpp>
#include <hamigaki/bjam/builtin_rules.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/noncopyable.hpp>

namespace hamigaki { namespace bjam {

class context : private boost::noncopyable
{
public:
    typedef variable_table::iterator variable_iterator;

    context()
    {
        frames_.push_back(frame(root_module_));

        list_of_list params;
        set_builtin_rule("ECHO", params, &builtins::echo);
        set_builtin_rule("Echo", params, &builtins::echo);
        set_builtin_rule("echo", params, &builtins::echo);

        params.push_back(boost::assign::list_of("messages")("*"));
        params.push_back(boost::assign::list_of("result-value")("?"));
        set_builtin_rule("EXIT", params, &builtins::exit);
        set_builtin_rule("Exit", params, &builtins::exit);
        set_builtin_rule("exit", params, &builtins::exit);
    }

    frame& current_frame()
    {
        return frames_.back();
    }

    void push_frame(const frame& f)
    {
        frames_.push_back(f);
    }

    void pop_frame()
    {
        frames_.pop_back();
    }

    void change_module(const boost::optional<std::string>& name)
    {
        frame& f = current_frame();

        if (f.module_name() == name)
            return;

        if (name)
            f.change_module(modules_[*name], name);
        else
            f.change_module(root_module_, name);
    }

    void set_builtin_rule(
        const std::string& name, const list_of_list& params,
        const boost::function1<void,context&>& func)
    {
        rule_def_ptr def(new rule_definition);

        def->parameters;
        def->native = func;

        root_module_.rules.set_rule_definition(name, def);
    }

    rule_def_ptr get_rule_definition(const std::string& name) const
    {
        const frame& f = frames_.back();
        const module& m = f.current_module();

        if (rule_def_ptr p = m.rules.get_rule_definition(name))
            return p;

        if (rule_def_ptr p = root_module_.rules.get_rule_definition(name))
            return p;

        return rule_def_ptr();
    }

    list_type invoke_rule(const std::string& name, const list_of_list& args);

private:
    module root_module_;
    std::map<std::string,module> modules_;
    std::vector<frame> frames_;
};

class scoped_change_module : private boost::noncopyable
{
public:
    scoped_change_module(
        bjam::context& ctx, const boost::optional<std::string>& name
    )
        : ctx_(ctx)
        , old_name_(ctx_.current_frame().module_name())
    {
        ctx_.change_module(name);
    }

    ~scoped_change_module()
    {
        ctx_.change_module(old_name_);
    }

private:
    bjam::context& ctx_;
    boost::optional<std::string> old_name_;
};

class scoped_push_frame : private boost::noncopyable
{
public:
    scoped_push_frame(bjam::context& ctx, const frame& f) : ctx_(ctx)
    {
        ctx_.push_frame(f);
    }

    ~scoped_push_frame()
    {
        ctx_.pop_frame();
    }

private:
    bjam::context& ctx_;
};

inline list_type
context::invoke_rule(const std::string& name, const list_of_list& args)
{
    rule_def_ptr rule = this->get_rule_definition(name);

    // TODO: may throw some exception
    if (!rule)
        return list_type();

    frame& old = current_frame();

    frame f(old.current_module(), old.module_name());
    f.rule_name(name);
    f.arguments() = args;

    scoped_push_frame guard(*this, f);
    this->change_module(rule->module_name);

    frame& cur_frame = current_frame();
    module& cur_module = cur_frame.current_module();

    variable_table local;
    const list_of_list& params = rule->parameters;
    for (std::size_t i = 0; i < params.size(); ++i)
        bjam::set_rule_argument(local, params[i], args[i]);

    scoped_push_local_variables using_local(cur_module.variables, local);

    if (rule->native)
        rule->native(*this);
    else
    {
        // Note: make a copy for the re-definition of the rule
        boost::shared_ptr<std::string> body = rule->body;
        const char* first = body->c_str();
        const char* last = first + body->size();

        typedef bjam::bjam_grammar_gen<const char*> grammar_type;
        grammar_type::parse_bjam_grammar(first, last, *this);
    }

    return current_frame().result();
}

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_BJAM_CONTEXT_HPP
