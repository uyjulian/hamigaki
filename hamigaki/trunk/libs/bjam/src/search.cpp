// search.cpp: search the target file

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/util/search.hpp>
#include <hamigaki/bjam/util/path.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

namespace hamigaki { namespace bjam {

HAMIGAKI_BJAM_DECL
void call_bind_rule(
    context& ctx, const std::string& name, const std::string& filename)
{
    frame& f = ctx.current_frame();
    module& m = f.current_module();

    const string_list& bindrule = m.variables.get_values("BINDRULE");
    if (!bindrule.empty())
    {
        const std::string& rule_name = bindrule[0];
        list_of_list args;
        args.push_back(boost::assign::list_of(name));
        args.push_back(boost::assign::list_of(filename));
        ctx.invoke_rule(rule_name, args);
    }
}

HAMIGAKI_BJAM_DECL
std::string search_target(context& ctx, const std::string& name)
{
    frame& f = ctx.current_frame();
    module& m = f.current_module();

    path_components compo;
    split_path(compo, name);
    compo.grist.clear();
    compo.member.clear();

    const string_list& locate = m.variables.get_values("LOCATE");
    const string_list& search_list = m.variables.get_values("SEARCH");

    bool found = false;
    std::string filename;

    if (!locate.empty())
    {
        compo.root = locate[0];
        filename = make_path(compo);
        found = true;
    }
    else if (!search_list.empty())
    {
        for (std::size_t i = 0, size = search_list.size(); i < size; ++i)
        {
            compo.root = search_list[i];
            filename = make_path(compo);

            if (fs::exists(fs::path(filename)))
            {
                found = true;
                break;
            }
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

    return filename;
}

} } // End namespaces bjam, hamigaki.
