// class.cpp: bjam class utilities

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/util/class.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/tuple/tuple.hpp>
#include <algorithm>

namespace hamigaki { namespace bjam {

namespace
{

void check_bases(const context& ctx, const string_list& bases)
{
    for (std::size_t i = 0, size = bases.size(); i < size; ++i)
    {
        if (!ctx.is_defined_module(bjam::class_module_name(bases[i])))
            throw std::runtime_error("the base class is undefined"); // FIXME
    }
}

bool is_localized_rule(
    const context& ctx,
    const rule_definition& def, const std::string& module_name)
{
    if (!def.module_name)
        return false;

    if (*def.module_name == module_name)
        return true;

    const module& m = ctx.get_module(*def.module_name);

    if (const std::string* p = m.class_module.get_ptr())
        return *p == module_name;
    else
        return false;
}

void import_bases(
    const context& ctx, module& cls_module,
    const std::string& module_name, const string_list& bases)
{
    typedef rule_table::iterator iter_type;

    for (std::size_t i = 0, size = bases.size(); i < size; ++i)
    {
        const std::string& base_mod_name = bjam::class_module_name(bases[i]);
        const module& m = ctx.get_module(base_mod_name);

        iter_type beg, end;
        boost::tie(beg, end) = m.rules.entries();
        for ( ; beg != end; ++beg)
        {
            std::string name = bases[i];
            name += '.';
            name += beg->first;

            rule_definition def = beg->second;
            if (is_localized_rule(ctx, def, base_mod_name))
                def.module_name = module_name;

            cls_module.rules.set_rule_definition(beg->first, def);
            cls_module.rules.set_rule_definition(name, def);
        }

        cls_module.imported_modules.insert(
            m.imported_modules.begin(),
            m.imported_modules.end()
        );
    }
}

} // namespace

HAMIGAKI_BJAM_DECL std::string class_module_name(const std::string& name)
{
    return "class@" + name;
}

HAMIGAKI_BJAM_DECL
std::string make_class(
    context& ctx, const std::string& name, const string_list& bases)
{
    check_bases(ctx, bases);

    const std::string& module_name = bjam::class_module_name(name);

    module& m = ctx.get_module(module_name);
    if (!m.variables.get_values("__name__").empty())
        throw already_defined_class(name);

    m.variables.set_values("__name__", string_list(name));
    m.variables.set_values("__bases__", bases);

    import_bases(ctx, m, module_name, bases);

    return module_name;
}

} } // End namespaces bjam, hamigaki.
