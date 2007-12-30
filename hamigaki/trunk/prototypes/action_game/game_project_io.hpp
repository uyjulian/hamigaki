// game_project_io.hpp: game project I/O

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef GAME_PROJECT_IO_HPP
#define GAME_PROJECT_IO_HPP

#include "game_project.hpp"

void save_game_project(const char* filename, const game_project& proj);
game_project load_game_project(const char* filename);

#endif // GAME_PROJECT_IO_HPP
