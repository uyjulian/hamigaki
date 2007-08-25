// bjam_grammar_test.cpp: a test driver for bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#define NOMINMAX
#include "../impl/bjam_preload.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>

using namespace boost::spirit;
namespace fs = boost::filesystem;

std::ostream& operator<<(std::ostream& os, const bjam_target& x)
{
    return os << x.name;
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: bjam_grammar_test (filename)" << std::endl;
            return 1;
        }

        std::string str;
        {
            std::ifstream is(argv[1], std::ios_base::binary);

            str.assign(
                std::istreambuf_iterator<char>(is),
                (std::istreambuf_iterator<char>())
            );
        }

        bjam_context ctx;
        ctx.working_directory =
            fs::system_complete(fs::path(argv[1], fs::no_check)).branch_path();

        variables vars;
        rule_table rules;
        ::bjam_preload(ctx.working_directory, vars, rules);

        bjam_grammar g(ctx, vars, rules);
        parse_info<const char*> info = parse(str.c_str(), g);

        if (!info.full)
        {
            std::cerr << "Error:" << std::endl;
            std::cerr << info.stop << std::endl;
            return 1;
        }

        std::sort(ctx.targets.begin(), ctx.targets.end());
        std::unique_copy(
            ctx.targets.begin(), ctx.targets.end(),
            std::ostream_iterator<bjam_target>(std::cout, "\n")
        );

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
