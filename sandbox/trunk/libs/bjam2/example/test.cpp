// test.cpp: test program for Hamigaki.Bjam

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#include <hamigaki/bjam2/grammars/bjam_grammar_gen.hpp>
#include <hamigaki/bjam2/bjam_interpreter.hpp>
#include <exception>
#include <fstream>
#include <iostream>

namespace bj = hamigaki::bjam2;
namespace bs = boost::spirit;

int main()
{
    try
    {
        std::ifstream is("test.jam");
        std::string src(
            std::istreambuf_iterator<char>(is),
            (std::istreambuf_iterator<char>())
        );
        is.close();

        const char* first = src.c_str();
        const char* last = first + src.size();

        typedef bj::bjam_grammar_gen<const char*> grammar_type;

        bj::tree_parse_info<> info =
            grammar_type::parse_bjam_grammar(first, last);

        if (!info.full)
        {
            std::cerr
                << "test.jam:" << info.line
                << ": syntax error at "
                << *info.stop
                << std::endl;
            return 1;
        }

        bj::context ctx;
        std::cout << bj::evaluate_bjam(ctx, info) << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}
