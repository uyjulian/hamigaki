//  bjam_parser.cpp: bjam parser

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#include "bjam_parser.hpp"
#include "impl/bjam_grammar.hpp"
#include <fstream>
#include <iterator>

bool parse_jamfile(
    const std::string& filename, std::vector<std::string>& targets)
{
    std::ifstream is(filename.c_str(), std::ios_base::binary);

    std::string src(
        std::istreambuf_iterator<char>(is),
        (std::istreambuf_iterator<char>())
    );

    bjam_grammar g(targets);
    bjam_skip_grammar skip;

    return boost::spirit::parse(src.c_str(), g, skip).full;
}
