// sprite_info_cache.hpp: the cache for sprite_info_set

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SPRITE_INFO_CACHE_HPP
#define SPRITE_INFO_CACHE_HPP

#include "sprite_info.hpp"
#include <boost/shared_ptr.hpp>
#include <string>

class sprite_info_cache
{
public:
    sprite_info_cache();
    ~sprite_info_cache();
    sprite_info_set& operator[](const std::string& filename);
    void clear();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // TEXTURE_CACHE_HPP
