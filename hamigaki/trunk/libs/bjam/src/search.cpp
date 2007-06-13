// search.cpp: search the target file

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/util/search.hpp>
#include <hamigaki/bjam/util/path.hpp>
#include <hamigaki/bjam/bjam_context.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

namespace hamigaki { namespace bjam {

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
            compo.root = locate[0];
            filename = make_path(compo);

            if (fs::exists(fs::path(filename, fs::native)))
            {
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        compo.root.clear();
        fs::path ph(make_path(compo), fs::native);
        fs::path work(ctx.working_directory(), fs::native);
        filename = fs::complete(ph, work).native_file_string();
    }

    return filename;
}

} } // End namespaces bjam, hamigaki.
