// file_types.cpp: utilities about file types

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/tools/release/ for library home page.

#include "file_types.hpp"

bool is_executable(const std::string& ext)
{
    return ext == ".pl";
}
