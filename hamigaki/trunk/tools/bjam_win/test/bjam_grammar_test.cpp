//  bjam_grammar_test.cpp: a test driver for bjam_grammar

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#include "../impl/bjam_grammar.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>

using namespace boost::spirit;
namespace fs = boost::filesystem;

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
        ctx.working_directory = fs::path(argv[1], fs::no_check).branch_path();

        variables vars;
        rule_table rules;
        bjam_grammar g(ctx, vars, rules);
        parse_info<const char*> info = parse(str.c_str(), g);

        if (!info.full)
        {
            std::cerr << "Error:" << std::endl;
            std::cerr << info.stop << std::endl;
            return 1;
        }

        std::copy(
            ctx.targets.begin(), ctx.targets.end(),
            std::ostream_iterator<std::string>(std::cout, "\n")
        );

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
