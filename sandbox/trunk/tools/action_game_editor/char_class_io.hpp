// char_class_io.hpp: character class I/O

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef CHAR_CLASS_IO_HPP
#define CHAR_CLASS_IO_HPP

#include "game_character_class.hpp"

void save_character_class(const char* filename, const game_character_class& c);
game_character_class load_character_class(const char* filename);

#endif // CHAR_CLASS_IO_HPP
