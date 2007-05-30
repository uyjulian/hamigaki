// bjam_check.cpp: bjam grammar checker

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

//#define BOOST_SPIRIT_DEBUG
#include <hamigaki/bjam/grammars/bjam_grammar.hpp>
#include <hamigaki/bjam/util/skip_parser.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace bjam = hamigaki::bjam;
using namespace boost::spirit;

std::string get_first_line(const char* s)
{
    const char* p = std::strchr(s, '\n');
    if (p)
    {
        if ((p != s) && (p[-1] == '\r'))
            --p;
        return std::string(s, p);
    }
    else
        return s;
}

void check_syntax(const char* filename)
{
    std::string str;
    {
        std::ifstream is(filename, std::ios_base::binary);

        str.assign(
            std::istreambuf_iterator<char>(is),
            (std::istreambuf_iterator<char>())
        );
    }

    bjam::context ctx;
    bjam::bjam_grammar g(ctx);
    bjam::skip_parser skip;
    parse_info<const char*> info = parse(str.c_str(), g, skip);

    if (!info.full)
    {
        throw std::runtime_error(
            "syntax error at \"" + get_first_line(info.stop) + '"');
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cerr << "Usage: bjam_check (filename) ..." << std::endl;
            return 1;
        }

        for (int i = 1; i < argc; ++i)
        {
            if (argc != 2)
                std::cout << argv[i] << std::endl;
            check_syntax(argv[i]);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
