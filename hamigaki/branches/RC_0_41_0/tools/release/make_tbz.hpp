// make_tbz.hpp: tar.bz2 archive writer for Hamigaki release tool

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/tools/release/ for library home page.

#ifndef MAKE_TBZ_HPP
#define MAKE_TBZ_HPP

#include <iosfwd>
#include <string>

void make_tbz2_archive(std::ostream& logs, const std::string& ver);

#endif // MAKE_TBZ_HPP
