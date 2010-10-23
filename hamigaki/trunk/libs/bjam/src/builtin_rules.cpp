// builtin_rules.cpp: bjam builtin rules

// Copyright Takeshi Mouri 2007-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#define NOMINMAX
#include <hamigaki/bjam/util/glob.hpp>
#include <hamigaki/bjam/util/path.hpp>
#include <hamigaki/bjam/util/regex.hpp>
#include <hamigaki/bjam/util/search.hpp>
#include <hamigaki/bjam/util/shell.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <hamigaki/bjam/builtin_rules.hpp>
#include <hamigaki/bjam/bjam_exceptions.hpp>
#include <hamigaki/checksum/md5.hpp>
#include <hamigaki/iterator/first_iterator.hpp>
#include <hamigaki/iterator/ostream_iterator.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/integer_traits.hpp>
#include <boost/next_prior.hpp>
#include <boost/regex.hpp>
#include <cstdlib>
#include <iomanip>
#include <locale>
#include <sstream>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    #include <hamigaki/bjam/util/win32/registry.hpp>
#endif

#if defined(BOOST_WINDOWS) && !defined(__CYGWIN__)
    #include <io.h>
#endif
#include <fcntl.h>

namespace fs = boost::filesystem;

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
        return x.second.exported;
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

    std::ostream& os = ctx.output_stream();

    std::copy(
        arg1.begin(), arg1.end(),
        hamigaki::ostream_iterator<std::string>(os, " ")
    );
    os << '\n';

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
        const std::string& pattern = bjam::convert_regex(regexps[j]);

        // Note: bjam's regex is not the same as "egrep" and "ECMAScript"
        boost::regex rex(pattern);
        for (std::size_t i = 0; i < list.size(); ++i)
        {
            boost::smatch what;
            if (regex_search(list[i], what, rex))
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

HAMIGAKI_BJAM_DECL string_list hdr_macro(context& ctx)
{
    // TODO: not implemented
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

HAMIGAKI_BJAM_DECL string_list update(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    string_list old = ctx.targets_to_update();
    ctx.targets_to_update(args[0]);
    return old;
}

HAMIGAKI_BJAM_DECL string_list update_now(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg2 = args[1];
    const string_list& arg3 = args[2];

    const string_list& targets = args[0];
    const boost::optional<std::string>& log = arg2.try_front();
    const boost::optional<std::string>& force = arg3.try_front();

    // TODO: not implemented

    return string_list(std::string("ok"));
}

HAMIGAKI_BJAM_DECL string_list subst(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg1 = args[0];

    const std::string& str = arg1[0];
    const std::string& pattern = bjam::convert_regex(arg1[1]);

    // Note: bjam's regex is not the same as "egrep" and "ECMAScript"
    boost::regex rex(pattern);

    string_list result;

    boost::smatch what;
    if (regex_search(str, what, rex))
    {
        for (std::size_t i = 2, size = arg1.size(); i < size; ++i)
            result += what.format(arg1[i]);
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

HAMIGAKI_BJAM_DECL string_list delete_module(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    module& m = ctx.get_module(args[0].try_front());
    m.rules.clear();
    m.variables.clear();

    return string_list();
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
        rule_definition def =
            src_module.rules.get_rule_definition(src_rules[i]);

        if (localize)
            def.module_name = tgt_module_name;
        def.exported = false;

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
        rule_definition* def = m.rules.get_rule_definition_ptr(rules[i]);
        if (!def)
            throw rule_not_found(rules[i]);

        def->exported = true;
    }

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list caller_module(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg1 = args[0];

    int level = 0;
    if (!arg1.empty())
        level = std::atoi(arg1[0].c_str());

    // skip root and current
    level += 2;
    if (level < 0)
        return string_list();

    const boost::optional<std::string>& name =
        ctx.caller_module_name(static_cast<std::size_t>(level));

    if (name)
        return string_list(*name);
    else
        return string_list();
}

HAMIGAKI_BJAM_DECL string_list back_trace(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();
    const string_list& arg1 = args[0];

    int level = boost::integer_traits<int>::const_max;
    if (!arg1.empty())
        level = std::atoi(arg1[0].c_str());

    if (level <= 0)
        return string_list();

    return ctx.back_trace(static_cast<std::size_t>(level), 1u);
}

HAMIGAKI_BJAM_DECL string_list pwd(context& ctx)
{
    return string_list(ctx.working_directory());
}

HAMIGAKI_BJAM_DECL string_list search_for_target(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& targets = args[0];
    const string_list& path = args[1];

    const std::string& name = targets[0];

    path_components compo;
    split_path(compo, name);
    compo.grist.clear();
    compo.member.clear();

    bool found = false;
    std::string filename;

    for (std::size_t i = 0, size = path.size(); i < size; ++i)
    {
        compo.root = path[i];
        filename = make_path(compo);

        if (fs::exists(fs::path(filename)))
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        compo.root.clear();
        fs::path ph(make_path(compo));
        fs::path work(ctx.working_directory());
        filename = fs::complete(ph, work).file_string();
    }

    call_bind_rule(ctx, name, filename);

    return string_list(name);
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

HAMIGAKI_BJAM_DECL string_list imported_modules(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    module& m = ctx.get_module(args[0].try_front());

    return string_list(
        m.imported_modules.begin(),
        m.imported_modules.end()
    );
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

HAMIGAKI_BJAM_DECL string_list native_rule(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const std::string& module_name = args[0][0];
    const std::string& rule_name = args[1][0];

    typedef std::map<std::string,bjam::native_rule> table_type;
    typedef table_type::const_iterator iter_type;

    module& m = ctx.get_module(module_name);
    const table_type& table = m.native_rules;
    iter_type it = table.find(rule_name);
    if (it == table.end())
        throw rule_not_found(rule_name);

    const bjam::native_rule& rule = it->second;

    rule_definition def;
    def.parameters = rule.parameters;
    def.native = rule.native;
    def.module_name = module_name;
    def.exported = true;
    m.rules.set_native_rule(rule_name, def);

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list has_native_rule(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    module& m = ctx.get_module(args[0][0]);
    const std::string& rule_name = args[1][0];
    int version = std::atoi(args[2][0].c_str());

    typedef std::map<std::string,bjam::native_rule> table_type;
    typedef table_type::const_iterator iter_type;

    const table_type& table = m.native_rules;
    iter_type it = table.find(rule_name);
    if ((it != table.end()) && (it->second.version == version))
        return string_list(std::string("true"));

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list user_module(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& modules = args[0];
    module& m = ctx.get_module(args[0].try_front());

    for (std::size_t i = 0, size = modules.size(); i < size; ++i)
    {
        module& m = ctx.get_module(modules[i]);
        m.user_module = true;
    }

    return string_list();
}

HAMIGAKI_BJAM_DECL string_list nearest_user_location(context& ctx)
{
    frame& f = ctx.current_frame();

    frame* user_frame =
        f.current_module().user_module ? &f : f.prev_user_frame();
    if (!user_frame)
        return string_list();

    string_list result;
    result.push_back(user_frame->filename());
    {
        std::ostringstream os;
        os.imbue(std::locale::classic());
        os << user_frame->line();
        result.push_back(os.str());
    }

    return result;
}

HAMIGAKI_BJAM_DECL string_list check_if_file(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const std::string& file = args[0][0];

    fs::path ph(file);
    fs::path work(ctx.working_directory());
    ph = fs::complete(ph, work);
    if (fs::is_regular(ph))
        return string_list(std::string("true"));
    else
        return string_list();
}

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
HAMIGAKI_BJAM_DECL string_list w32_getreg(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& arg1 = args[0];
    const string_list& arg2 = args[1];

    const std::string& path = arg1[0];
    boost::optional<std::string> name = arg2.try_front();

    return win32::registry_values(path, name);
}

HAMIGAKI_BJAM_DECL string_list w32_getregnames(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const string_list& arg1 = args[0];
    const string_list& arg2 = args[1];

    const std::string& path = arg1[0];
    const std::string& result_type = arg2[0];

    if (result_type == "subkeys")
        return win32::registry_subkey_names(path);
    else if (result_type == "values")
        return win32::registry_value_names(path);
    else
        return string_list();
}
#endif

HAMIGAKI_BJAM_DECL string_list shell(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const std::string& cmd = args[0][0];
    const string_list& arg2 = args[1];

    bool need_status = false;
    bool need_capture = true;
    for (std::size_t i = 0, size = arg2.size(); i < size; ++i)
    {
        if (arg2[i] == "exit-status")
            need_status = true;
        else if (arg2[i] == "no-output")
            need_capture = false;
    }

    return bjam::shell(cmd, need_status, need_capture);
}

HAMIGAKI_BJAM_DECL string_list md5(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const std::string& s = args[0][0];

    checksum::md5 md5;
    md5.process_bytes(s.c_str(), s.size());

    typedef checksum::md5::value_type value_type;
    const value_type& v = md5.checksum();

    std::ostringstream os;
    os.imbue(std::locale::classic());
    os << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < value_type::static_size; ++i)
        os << std::setw(2) << static_cast<unsigned>(v[i]);

    return string_list(os.str());
}

HAMIGAKI_BJAM_DECL string_list file_open(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    const std::string& name = args[0][0];
    const std::string& mode = args[1][0];

    int fd;
    if (mode == "w")
        fd = ::open(name.c_str(), O_WRONLY|O_CREAT|O_TRUNC, 0666);
    else
        fd = ::open(name.c_str(), O_RDONLY);

    if (fd == -1)
        return string_list();

    std::ostringstream os;
    os.imbue(std::locale::classic());
    os << fd;
    return string_list(os.str());
}

HAMIGAKI_BJAM_DECL string_list pad(context& ctx)
{
    frame& f = ctx.current_frame();
    const list_of_list& args = f.arguments();

    std::string str = args[0][0];
    std::size_t width = static_cast<std::size_t>(std::atoi(args[1][0].c_str()));

    if (str.size() < width)
        str.resize(width, ' ');

    return string_list(str);
}

HAMIGAKI_BJAM_DECL string_list precious(context& ctx)
{
    set_target_flags(ctx, target::precious);
    return string_list();
}

HAMIGAKI_BJAM_DECL string_list self_path(context& ctx)
{
    // FIXME
    return string_list("bjam");
}

} // namespace builtins

HAMIGAKI_BJAM_DECL void set_builtin_rules(context& ctx)
{
    list_of_list params;

    params.clear();
    ctx.set_builtin_rule("ALWAYS", params, &builtins::always);
    ctx.set_builtin_rule("Always", params, &builtins::always, false);

    params.clear();
    ctx.set_builtin_rule("DEPENDS", params, &builtins::depends);
    ctx.set_builtin_rule("Depends", params, &builtins::depends, false);

    params.clear();
    ctx.set_builtin_rule("ECHO", params, &builtins::echo);
    ctx.set_builtin_rule("Echo", params, &builtins::echo, false);
    ctx.set_builtin_rule("echo", params, &builtins::echo, false);

    params.push_back(boost::assign::list_of("messages")("*"));
    params.push_back(boost::assign::list_of("result-value")("?"));
    ctx.set_builtin_rule("EXIT", params, &builtins::exit);
    ctx.set_builtin_rule("Exit", params, &builtins::exit, false);
    ctx.set_builtin_rule("exit", params, &builtins::exit, false);

    params.clear();
    params.push_back(boost::assign::list_of("directories")("*"));
    params.push_back(boost::assign::list_of("patterns")("*"));
    params.push_back(boost::assign::list_of("case-insensitive")("?"));
    ctx.set_builtin_rule("GLOB", params, &builtins::glob);
    ctx.set_builtin_rule("Glob", params, &builtins::glob, false);

    params.clear();
    params.push_back(boost::assign::list_of("patterns")("*"));
    ctx.set_builtin_rule("GLOB-RECURSIVELY", params, &builtins::glob_recursive);

    params.clear();
    ctx.set_builtin_rule("INCLUDES", params, &builtins::includes);
    ctx.set_builtin_rule("Includes", params, &builtins::includes, false);

    params.clear();
    params.push_back(boost::assign::list_of("targets")("*"));
    params.push_back(boost::assign::list_of("targets-to-rebuild")("*"));
    ctx.set_builtin_rule("REBUILDS", params, &builtins::rebuilds);

    params.clear();
    ctx.set_builtin_rule("LEAVES", params, &builtins::leaves);
    ctx.set_builtin_rule("Leaves", params, &builtins::leaves, false);

    params.clear();
    ctx.set_builtin_rule("MATCH", params, &builtins::match);
    ctx.set_builtin_rule("Match", params, &builtins::match, false);

    params.clear();
    ctx.set_builtin_rule("NOCARE", params, &builtins::no_care);
    ctx.set_builtin_rule("NoCare", params, &builtins::no_care, false);

    params.clear();
    ctx.set_builtin_rule("NOTFILE", params, &builtins::not_file);
    ctx.set_builtin_rule("NotFile", params, &builtins::not_file, false);
    ctx.set_builtin_rule("NOTIME", params, &builtins::not_file, false);

    params.clear();
    ctx.set_builtin_rule("NOUPDATE", params, &builtins::no_update);
    ctx.set_builtin_rule("NoUpdate", params, &builtins::no_update, false);

    params.clear();
    ctx.set_builtin_rule("TEMPORARY", params, &builtins::temporary);
    ctx.set_builtin_rule("Temporary", params, &builtins::temporary, false);

    params.clear();
    params.push_back(boost::assign::list_of("targets")("*"));
    ctx.set_builtin_rule("ISFILE", params, &builtins::is_file);

    params.clear();
    ctx.set_builtin_rule("HDRMACRO", params, &builtins::hdr_macro);
    ctx.set_builtin_rule("HdrMacro", params, &builtins::hdr_macro, false);

    params.clear();
    ctx.set_builtin_rule("FAIL_EXPECTED", params, &builtins::fail_expected);

    params.clear();
    ctx.set_builtin_rule("RMOLD", params, &builtins::rm_old);

    params.clear();
    params.push_back(
        boost::assign::list_of("targets")("*"));
    ctx.set_builtin_rule("UPDATE", params, &builtins::update);

    params.clear();
    params.push_back(boost::assign::list_of("targets")("*"));
    params.push_back(boost::assign::list_of("log")("?"));
    params.push_back(boost::assign::list_of("ignore-minus-n")("?"));
    ctx.set_builtin_rule("UPDATE_NOW", params, &builtins::update_now);

    params.clear();
    params.push_back(
        boost::assign::list_of("string")("pattern")("replacements")("+"));
    ctx.set_builtin_rule("SUBST", params, &builtins::subst);
    ctx.set_builtin_rule("subst", params, &builtins::subst, false);

    params.clear();
    params.push_back(boost::assign::list_of("module")("?"));
    ctx.set_builtin_rule("RULENAMES", params, &builtins::rule_names);
    ctx.set_builtin_rule("VARNAMES", params, &builtins::var_names);
    ctx.set_builtin_rule("DELETE_MODULE", params, &builtins::delete_module);

    params.clear();
    params.push_back(boost::assign::list_of("source_module")("?"));
    params.push_back(boost::assign::list_of("source_rules")("*"));
    params.push_back(boost::assign::list_of("target_module")("?"));
    params.push_back(boost::assign::list_of("target_rules")("*"));
    params.push_back(boost::assign::list_of("localize")("?"));
    ctx.set_builtin_rule("IMPORT", params, &builtins::import);

    params.clear();
    params.push_back(boost::assign::list_of("module")("?"));
    params.push_back(boost::assign::list_of("rules")("*"));
    ctx.set_builtin_rule("EXPORT", params, &builtins::export_);

    params.clear();
    params.push_back(boost::assign::list_of("levels")("?"));
    ctx.set_builtin_rule("CALLER_MODULE", params, &builtins::caller_module);
    ctx.set_builtin_rule("BACKTRACE", params, &builtins::back_trace);

    params.clear();
    ctx.set_builtin_rule("PWD", params, &builtins::pwd);

    params.clear();
    params.push_back(boost::assign::list_of("target")("*"));
    params.push_back(boost::assign::list_of("path")("*"));
    ctx.set_builtin_rule(
        "SEARCH_FOR_TARGET", params, &builtins::search_for_target);

    params.clear();
    params.push_back(boost::assign::list_of("modules_to_import")("+"));
    params.push_back(boost::assign::list_of("target_module")("?"));
    ctx.set_builtin_rule("IMPORT_MODULE", params, &builtins::import_module);

    params.clear();
    params.push_back(boost::assign::list_of("module")("?"));
    ctx.set_builtin_rule(
        "IMPORTED_MODULES", params, &builtins::imported_modules);

    params.clear();
    params.push_back(boost::assign::list_of("instance_module"));
    params.push_back(boost::assign::list_of("class_module"));
    ctx.set_builtin_rule("INSTANCE", params, &builtins::instance);

    params.clear();
    params.push_back(boost::assign::list_of("sequence")("*"));
    ctx.set_builtin_rule("SORT", params, &builtins::sort);

    params.clear();
    params.push_back(boost::assign::list_of("path_parts")("*"));
    ctx.set_builtin_rule("NORMALIZE_PATH", params, &builtins::normalize_path);

    params.clear();
    params.push_back(boost::assign::list_of("args")("*"));
    ctx.set_builtin_rule("CALC", params, &builtins::calc);

    params.clear();
    params.push_back(boost::assign::list_of("module"));
    params.push_back(boost::assign::list_of("rule"));
    ctx.set_builtin_rule("NATIVE_RULE", params, &builtins::native_rule);
    params.push_back(boost::assign::list_of("version"));
    ctx.set_builtin_rule("HAS_NATIVE_RULE", params, &builtins::has_native_rule);

    params.clear();
    params.push_back(boost::assign::list_of("module")("*"));
    ctx.set_builtin_rule("USER_MODULE", params, &builtins::user_module);

    params.clear();
    ctx.set_builtin_rule(
        "NEAREST_USER_LOCATION", params, &builtins::nearest_user_location);

    params.clear();
    params.push_back(boost::assign::list_of("file"));
    ctx.set_builtin_rule("CHECK_IF_FILE", params, &builtins::check_if_file);

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)
    params.clear();
    params.push_back(boost::assign::list_of("key_path"));
    params.push_back(boost::assign::list_of("data")("?"));
    ctx.set_builtin_rule("W32_GETREG", params, &builtins::w32_getreg);

    params.clear();
    params.push_back(boost::assign::list_of("key_path"));
    params.push_back(boost::assign::list_of("result-type"));
    ctx.set_builtin_rule("W32_GETREGNAMES", params, &builtins::w32_getregnames);
#endif

    params.clear();
    params.push_back(boost::assign::list_of("command"));
    params.push_back(boost::assign::list_of("*"));
    ctx.set_builtin_rule("SHELL", params, &builtins::shell);
    ctx.set_builtin_rule("COMMAND", params, &builtins::shell);

    params.clear();
    params.push_back(boost::assign::list_of("string"));
    ctx.set_builtin_rule("MD5", params, &builtins::md5);

    params.clear();
    params.push_back(boost::assign::list_of("name"));
    params.push_back(boost::assign::list_of("mode"));
    ctx.set_builtin_rule("FILE_OPEN", params, &builtins::file_open);

    params.clear();
    params.push_back(boost::assign::list_of("string"));
    params.push_back(boost::assign::list_of("width"));
    ctx.set_builtin_rule("PAD", params, &builtins::pad);

    params.clear();
    params.push_back(boost::assign::list_of("targets")("*"));
    ctx.set_builtin_rule("PRECIOUS", params, &builtins::precious);

    params.clear();
    ctx.set_builtin_rule("SELF_PATH", params, &builtins::self_path);
}

} } // End namespaces bjam, hamigaki.
