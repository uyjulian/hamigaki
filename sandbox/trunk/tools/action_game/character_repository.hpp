// character_repository.hpp: the repository for character classes

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef CHARACTER_REPOSITORY_HPP
#define CHARACTER_REPOSITORY_HPP

#include "game_character_class.hpp"
#include <boost/shared_ptr.hpp>

class character_repository
{
public:
    character_repository();
    ~character_repository();
    const game_character_class& operator[](const hamigaki::uuid& id) const;

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // CHARACTER_REPOSITORY_HPP
