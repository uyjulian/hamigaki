// instantiate_bjam_grammar.cpp: instantiation of bjam_grammar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/grammars/bjam_grammar.hpp>

template struct ::hamigaki::bjam::bjam_grammar_gen<const char*>;
