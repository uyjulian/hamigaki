// bjam_context.hpp: the context information for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_BJAM_CONTEXT_HPP
#define HAMIGAKI_BJAM_BJAM_CONTEXT_HPP

#include <hamigaki/bjam/bjam_config.hpp>
#include <hamigaki/bjam/util/frame.hpp>
#include <hamigaki/bjam/util/target.hpp>
#include <list>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251 4275)
#endif

namespace hamigaki { namespace bjam {

class context;

HAMIGAKI_BJAM_DECL void set_predefined_variables(context& ctx);
HAMIGAKI_BJAM_DECL void set_builtin_rules(context& ctx);

class HAMIGAKI_BJAM_DECL context : private boost::noncopyable
{
private:
    typedef std::list<frame> frame_stack;

public:
    typedef variable_table::iterator variable_iterator;

    context();

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

    boost::optional<std::string> caller_module_name(std::size_t level) const;
    string_list back_trace(std::size_t level, std::size_t skip=0) const;

    module& get_module(const boost::optional<std::string>& name);
    const module& get_module(const boost::optional<std::string>& name) const;

    bool is_defined_module(const std::string& name) const
    {
        return modules_.find(name) != modules_.end();
    }

    void change_module(const boost::optional<std::string>& name);

    target& get_target(const std::string& name)
    {
        return targets_[name];
    }

    const string_list& targets_to_update() const
    {
        return targets_to_update_;
    }

    void targets_to_update(const string_list& x)
    {
        targets_to_update_ = x;
    }

    void set_native_rule(
        const std::string& name, const list_of_list& params,
        const boost::function1<string_list,context&>& func,
        bool exported = true);

    rule_def_ptr get_rule_definition(const std::string& name) const;
    string_list invoke_rule(const std::string& name, const list_of_list& args);

    std::string working_directory() const
    {
        return working_directory_;
    }

    void working_directory(const std::string& dir)
    {
        working_directory_ = dir;
    }

private:
    module root_module_;
    std::map<std::string,module> modules_;
    std::map<std::string,target> targets_;
    string_list targets_to_update_;
    frame_stack frames_;
    std::string working_directory_;

    rule_def_ptr get_imported_rule_definition(
        const std::string& module_name, const std::string& rule_name) const;
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

class scoped_on_target : private boost::noncopyable
{
public:
    scoped_on_target(
        bjam::context& ctx, const std::string& name
    )
        : ctx_(ctx), name_(name)
    {
        module& m = ctx_.current_frame().current_module();
        m.variables.push_local_variables(ctx_.get_target(name_).variables);
    }

    ~scoped_on_target()
    {
        module& m = ctx_.current_frame().current_module();
        m.variables.pop_local_variables(ctx_.get_target(name_).variables);
    }

private:
    bjam::context& ctx_;
    std::string name_;
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

} } // End namespaces bjam, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM_BJAM_CONTEXT_HPP
