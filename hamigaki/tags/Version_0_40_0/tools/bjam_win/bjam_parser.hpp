// bjam_parser.hpp: bjam parser

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef BJAM_PARSER_HPP
#define BJAM_PARSER_HPP

#include "impl/bjam_target.hpp"

bool parse_jamfile(
    const std::string& filename, std::vector<bjam_target>& targets);

#endif // BJAM_PARSER_HPP
