// builtin_rules.cpp: bjam builtin rules

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#define NOMINMAX
#include <hamigaki/bjam/util/glob.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <hamigaki/bjam/builtin_rules.hpp>
#include <hamigaki/bjam/bjam_exceptions.hpp>
#include <hamigaki/iterator/first_iterator.hpp>
#include <hamigaki/iterator/ostream_iterator.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/next_prior.hpp>
#include <boost/regex.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace hamigaki { namespace bjam {

namespace builtins
{

namespace
{

struct is_exported
{
    typedef rule_table::table_type::value_type value_type;

    bool operator()(const value_type& x) const
    {
        return x.second->exported;
    }
};

} // namespace

HAMIGAKI_BJAM_DECL string_list echo(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg1 = args[0];

    std::copy(
        arg1.begin(), arg1.end(),
        hamigaki::ostream_iterator<std::string>(std::cout, " ")
    );
    std::cout << '\n';

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list exit(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg1 = args[0];
    const string_list& arg2 = args[1];

    std::ostringstream os;
    std::copy(
        arg1.begin(), arg1.end(),
        hamigaki::ostream_iterator<std::string>(os, " ")
    );

    int code = EXIT_FAILURE;
    if (!arg2.empty())
        code = std::atoi(arg2[0].c_str()); // FIXME

    throw exit_exception(os.str(), code);

    BOOST_UNREACHABLE_RETURN(string_list())
}

HAMIGAKI_BJAM_DECL string_list glob(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& dirs = args[0];
    const string_list& patterns = args[1];
    const bool flag = !args[2].empty();

    string_list result;

    for (std::size_t i = 0; i < dirs.size(); ++i)
    {
        for (std::size_t j = 0; j < patterns.size(); ++j)
        {
            result += bjam::glob(
                ctx.working_directory(), dirs[i], patterns[j], flag);
        }
    }

    return result;
}

HAMIGAKI_BJAM_DECL string_list glob_recursive(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& patterns = args[0];

    string_list result;

    for (std::size_t i = 0; i < patterns.size(); ++i)
    {
        result += bjam::glob_recursive(
            ctx.working_directory(), patterns[i]);
    }

    return result;
}

HAMIGAKI_BJAM_DECL string_list match(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& regexps = args[0];
    const string_list& list = args[1];

    string_list result;

    for (std::size_t j = 0; j < regexps.size(); ++j)
    {
        // Note: bjam's regex is not the same as "extended" and "egrep"
        boost::regex rex(regexps[j], boost::regex_constants::extended);
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            boost::smatch what;
            if (regex_match(list[i], what, rex))
            {
                result.insert(
                    result.end(), boost::next(what.begin()), what.end());
            }
        }
    }

    return result;
}

HAMIGAKI_BJAM_DECL string_list rulenames(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg1 = args[0];

    boost::optional<std::string> name;
    if (!arg1.empty())
        name = arg1[0];

    module& m = ctx.get_module(name);

    rule_table::iterator beg, end;
    boost::tie(beg, end) = m.rules.entries();

    return string_list(
        make_first_iterator(boost::make_filter_iterator<is_exported>(beg, end)),
        make_first_iterator(boost::make_filter_iterator<is_exported>(end, end))
    );
}

HAMIGAKI_BJAM_DECL string_list varnames(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg1 = args[0];

    boost::optional<std::string> name;
    if (!arg1.empty())
        name = arg1[0];

    module& m = ctx.get_module(name);

    variable_table::iterator beg, end;
    boost::tie(beg, end) = m.variables.entries();

    return string_list(
        make_first_iterator(beg),
        make_first_iterator(end)
    );
}

HAMIGAKI_BJAM_DECL string_list import(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    module& src_module = ctx.get_module(args[0].try_front());
    const string_list& src_rules = args[1];
    const boost::optional<std::string>& tgt_module_name = args[2].try_front();
    module& tgt_module = ctx.get_module(tgt_module_name);
    const string_list& tgt_rules = args[3];
    const bool localize = !args[4].empty();

    if (src_rules.size() != tgt_rules.size())
        throw std::runtime_error("the count of rule names mismatch"); // FIXME

    for (std::size_t i = 0, size = src_rules.size(); i < size; ++i)
    {
        rule_def_ptr src = src_module.rules.get_rule_definition(src_rules[i]);
        if (!src)
            throw std::runtime_error("rule not found"); // FIXME

        rule_def_ptr def(new rule_definition(*src));
        if (localize)
            def->module_name = tgt_module_name;
        def->exported = false;

        tgt_module.rules.set_rule_definition(tgt_rules[i], def);
    }

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list export_(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    module& m = ctx.get_module(args[0].try_front());
    const string_list& rules = args[1];

    for (std::size_t i = 0, size = rules.size(); i < size; ++i)
    {
        rule_def_ptr def = m.rules.get_rule_definition(rules[i]);
        if (!def)
            throw std::runtime_error("rule not found"); // FIXME

        def->exported = true;
    }

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list import_module(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& imported = args[0];
    module& tgt_module = ctx.get_module(args[1].try_front());

    tgt_module.imported_modules.insert(imported.begin(), imported.end());

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list instance(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    module& instance_module = ctx.get_module(args[0][0]);
    instance_module.class_module = args[1][0];

    return string_list();
}

} // namespace builtins

HAMIGAKI_BJAM_DECL void set_builtin_rules(context& ctx)
{
    list_of_list params;
    ctx.set_native_rule("ECHO", params, &builtins::echo);
    ctx.set_native_rule("Echo", params, &builtins::echo, false);
    ctx.set_native_rule("echo", params, &builtins::echo, false);

    params.push_back(boost::assign::list_of("messages")("*"));
    params.push_back(boost::assign::list_of("result-value")("?"));
    ctx.set_native_rule("EXIT", params, &builtins::exit);
    ctx.set_native_rule("Exit", params, &builtins::exit, false);
    ctx.set_native_rule("exit", params, &builtins::exit, false);

    params.clear();
    params.push_back(boost::assign::list_of("directories")("*"));
    params.push_back(boost::assign::list_of("patterns")("*"));
    params.push_back(boost::assign::list_of("case-insensitive")("?"));
    ctx.set_native_rule("GLOB", params, &builtins::glob);
    ctx.set_native_rule("Glob", params, &builtins::glob, false);

    params.clear();
    params.push_back(boost::assign::list_of("patterns")("*"));
    ctx.set_native_rule("GLOB-RECURSIVELY", params, &builtins::glob_recursive);

    params.clear();
    ctx.set_native_rule("MATCH", params, &builtins::match);
    ctx.set_native_rule("Match", params, &builtins::match, false);

    params.clear();
    params.push_back(boost::assign::list_of("module")("?"));
    ctx.set_native_rule("RULENAMES", params, &builtins::rulenames);
    ctx.set_native_rule("VARNAMES", params, &builtins::varnames);

    params.clear();
    params.push_back(boost::assign::list_of("source_module")("?"));
    params.push_back(boost::assign::list_of("source_rules")("*"));
    params.push_back(boost::assign::list_of("target_module")("?"));
    params.push_back(boost::assign::list_of("target_rules")("*"));
    params.push_back(boost::assign::list_of("localize")("?"));
    ctx.set_native_rule("IMPORT", params, &builtins::import);

    params.clear();
    params.push_back(boost::assign::list_of("module")("?"));
    params.push_back(boost::assign::list_of("rules")("*"));
    ctx.set_native_rule("EXPORT", params, &builtins::export_);

    params.clear();
    params.push_back(boost::assign::list_of("modules_to_import")("+"));
    params.push_back(boost::assign::list_of("target_module")("?"));
    ctx.set_native_rule("IMPORT_MODULE", params, &builtins::import_module);

    params.clear();
    params.push_back(boost::assign::list_of("instance_module"));
    params.push_back(boost::assign::list_of("class_module"));
    ctx.set_native_rule("INSTANCE", params, &builtins::instance);
}

} } // End namespaces bjam, hamigaki.
