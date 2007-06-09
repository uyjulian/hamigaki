// builtin_rules.cpp: bjam builtin rules

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#define NOMINMAX
#include <hamigaki/bjam/bjam_context.hpp>
#include <hamigaki/bjam/builtin_rules.hpp>
#include <hamigaki/bjam/bjam_exceptions.hpp>
#include <hamigaki/iterator/first_iterator.hpp>
#include <hamigaki/iterator/ostream_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace hamigaki { namespace bjam { namespace builtins {

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

} } } // End namespaces builtins, bjam, hamigaki.
