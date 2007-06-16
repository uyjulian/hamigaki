// builtin_rules.cpp: bjam builtin rules

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#define NOMINMAX
#include <hamigaki/bjam/util/glob.hpp>
#include <hamigaki/bjam/util/path.hpp>
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
#include <locale>
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

inline void set_target_flags(context& ctx, unsigned flag)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& targets = args[0];

    for (std::size_t i = 0; i < targets.size(); ++i)
        ctx.get_target(targets[i]).flags |= flag;
}

} // namespace

HAMIGAKI_BJAM_DECL string_list always(context& ctx)
{
    set_target_flags(ctx, target::force_update);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list depends(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& targets = args[0];
    const string_list& sources = args[1];

    for (std::size_t i = 0, size = targets.size(); i < size; ++i)
    {
        target& t = ctx.get_target(targets[i]);
        t.depended_targets.insert(sources.begin(), sources.end());
    }

    return string_list();
}

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

HAMIGAKI_BJAM_DECL string_list includes(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& targets = args[0];
    const string_list& sources = args[1];

    for (std::size_t i = 0, size = targets.size(); i < size; ++i)
    {
        target& t = ctx.get_target(targets[i]);
        t.included_targets.insert(sources.begin(), sources.end());
    }

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list rebuilds(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& targets = args[0];
    const string_list& sources = args[1];

    for (std::size_t i = 0, size = targets.size(); i < size; ++i)
    {
        target& t = ctx.get_target(targets[i]);
        t.rebuilt_targets.insert(sources.begin(), sources.end());
    }

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list leaves(context& ctx)
{
    set_target_flags(ctx, target::leaves);
    return string_list();
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

HAMIGAKI_BJAM_DECL string_list no_care(context& ctx)
{
    set_target_flags(ctx, target::no_care);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list not_file(context& ctx)
{
    set_target_flags(ctx, target::not_file);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list no_update(context& ctx)
{
    set_target_flags(ctx, target::no_update);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list temporary(context& ctx)
{
    set_target_flags(ctx, target::temporary);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list is_file(context& ctx)
{
    set_target_flags(ctx, target::is_file);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list fail_expected(context& ctx)
{
    set_target_flags(ctx, target::fail_expected);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list rm_old(context& ctx)
{
    set_target_flags(ctx, target::rm_old);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list subst(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg0 = args[0];

    const std::string& str = arg0[0];
    const std::string& pattern = arg0[1];

    // Note: bjam's regex is not the same as "extended" and "egrep"
    boost::regex rex(pattern, boost::regex_constants::extended);

    string_list result;

    boost::smatch what;
    if (regex_match(str, what, rex))
    {
        for (std::size_t i = 2, size = arg0.size(); i < size; ++i)
            result += what.format(arg0[i]);
    }

    return result;
}

HAMIGAKI_BJAM_DECL string_list rule_names(context& ctx)
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

HAMIGAKI_BJAM_DECL string_list var_names(context& ctx)
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

HAMIGAKI_BJAM_DECL string_list pwd(context& ctx)
{
    return string_list(ctx.working_directory());
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

HAMIGAKI_BJAM_DECL string_list sort(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    string_list sequence = args[0];
    sequence.sort();

    return sequence;
}

HAMIGAKI_BJAM_DECL string_list normalize_path(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    return string_list(bjam::normalize_path(args[0]));
}

HAMIGAKI_BJAM_DECL string_list calc(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& arg1 = args[0];

    if (arg1.size() < 3u)
        return string_list();

    long lhs = arg1[0].empty() ? 0 : std::atoi(arg1[0].c_str()); // FIXME
    char op = arg1[1][0];
    long rhs = arg1[2].empty() ? 0 : std::atoi(arg1[2].c_str()); // FIXME

    long value;
    if (op == '+')
        value = lhs + rhs;
    else if (op == '-')
        value = lhs - rhs;
    else
        return string_list();

    std::ostringstream os;
    os.imbue(std::locale::classic());
    os << value;
    return string_list(os.str());
}

} // namespace builtins

HAMIGAKI_BJAM_DECL void set_builtin_rules(context& ctx)
{
    list_of_list params;

    params.clear();
    ctx.set_native_rule("ALWAYS", params, &builtins::always);
    ctx.set_native_rule("Always", params, &builtins::always, false);

    params.clear();
    ctx.set_native_rule("DEPENDS", params, &builtins::depends);
    ctx.set_native_rule("Depends", params, &builtins::depends, false);

    params.clear();
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
    ctx.set_native_rule("INCLUDES", params, &builtins::includes);
    ctx.set_native_rule("Includes", params, &builtins::includes, false);

    params.clear();
    params.push_back(boost::assign::list_of("targets")("*"));
    params.push_back(boost::assign::list_of("targets-to-rebuild")("*"));
    ctx.set_native_rule("REBUILDS", params, &builtins::rebuilds);

    params.clear();
    ctx.set_native_rule("LEAVES", params, &builtins::leaves);
    ctx.set_native_rule("Leaves", params, &builtins::leaves, false);

    params.clear();
    ctx.set_native_rule("MATCH", params, &builtins::match);
    ctx.set_native_rule("Match", params, &builtins::match, false);

    params.clear();
    ctx.set_native_rule("NOCARE", params, &builtins::no_care);
    ctx.set_native_rule("NoCare", params, &builtins::no_care, false);

    params.clear();
    ctx.set_native_rule("NOTFILE", params, &builtins::not_file);
    ctx.set_native_rule("NotFile", params, &builtins::not_file, false);
    ctx.set_native_rule("NOTIME", params, &builtins::not_file, false);

    params.clear();
    ctx.set_native_rule("NOUPDATE", params, &builtins::no_update);
    ctx.set_native_rule("NoUpdate", params, &builtins::no_update, false);

    params.clear();
    ctx.set_native_rule("TEMPORARY", params, &builtins::temporary);
    ctx.set_native_rule("Temporary", params, &builtins::temporary, false);

    params.clear();
    params.push_back(boost::assign::list_of("targets")("*"));
    ctx.set_native_rule("ISFILE", params, &builtins::is_file);

    params.clear();
    ctx.set_native_rule("FAIL_EXPECTED", params, &builtins::fail_expected);

    params.clear();
    ctx.set_native_rule("RMOLD", params, &builtins::rm_old);

    params.clear();
    params.push_back(
        boost::assign::list_of("string")("pattern")("replacements")("+"));
    ctx.set_native_rule("SUBST", params, &builtins::subst);
    ctx.set_native_rule("subst", params, &builtins::subst, false);

    params.clear();
    params.push_back(boost::assign::list_of("module")("?"));
    ctx.set_native_rule("RULENAMES", params, &builtins::rule_names);
    ctx.set_native_rule("VARNAMES", params, &builtins::var_names);

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
    ctx.set_native_rule("PWD", params, &builtins::pwd);

    params.clear();
    params.push_back(boost::assign::list_of("modules_to_import")("+"));
    params.push_back(boost::assign::list_of("target_module")("?"));
    ctx.set_native_rule("IMPORT_MODULE", params, &builtins::import_module);

    params.clear();
    params.push_back(boost::assign::list_of("instance_module"));
    params.push_back(boost::assign::list_of("class_module"));
    ctx.set_native_rule("INSTANCE", params, &builtins::instance);

    params.clear();
    params.push_back(boost::assign::list_of("sequence")("*"));
    ctx.set_native_rule("SORT", params, &builtins::sort);

    params.clear();
    params.push_back(boost::assign::list_of("path_parts")("*"));
    ctx.set_native_rule("NORMALIZE_PATH", params, &builtins::normalize_path);

    params.clear();
    params.push_back(boost::assign::list_of("args")("*"));
    ctx.set_native_rule("CALC", params, &builtins::calc);
}

} } // End namespaces bjam, hamigaki.
