// bjam_parser.cpp: bjam parser

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "bjam_parser.hpp"
#include "impl/bjam_preload.hpp"

namespace fs = boost::filesystem;

bool parse_jamfile(
    const std::string& filename, std::vector<bjam_target>& targets)
{
    std::ifstream is(filename.c_str(), std::ios_base::binary);
    if (!is)
        return false;

    std::string src(
        std::istreambuf_iterator<char>(is),
        (std::istreambuf_iterator<char>())
    );

    bjam_context ctx;
    ctx.working_directory = fs::path(filename, fs::no_check).branch_path();

    variables vars;
    rule_table rules;

    ::bjam_preload(ctx.working_directory, vars, rules);

    bjam_grammar g(ctx, vars, rules);

    if (boost::spirit::parse(src.c_str(), g).full)
    {
        targets.swap(ctx.targets);
        std::sort(targets.begin(), targets.end());

        targets.erase(
            std::unique(targets.begin(), targets.end()),
            targets.end()
        );

        return true;
    }
    else
        return false;
}
