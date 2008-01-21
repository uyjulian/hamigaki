// sprite_info_cache.cpp: the cache for sprite_info_set

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "sprite_info_cache.hpp"
#include <map>

class sprite_info_cache::impl
{
private:
    typedef std::map<std::string,sprite_info_set> table_type;

public:
    sprite_info_set& operator[](const std::string& filename)
    {
        table_type::iterator pos = table_.lower_bound(filename);
        if ((pos == table_.end()) || (pos->first > filename))
        {
            sprite_info_set infos;
            load_sprite_info_set(filename.c_str(), infos);

            pos = table_.insert(
                pos, table_type::value_type(filename, sprite_info_set())
            );
            std::swap(pos->second, infos);
        }
        return pos->second;
    }

    void clear()
    {
        table_.clear();
    }

private:
    std::map<std::string,sprite_info_set> table_;
};

sprite_info_cache::sprite_info_cache() : pimpl_(new impl)
{
}

sprite_info_cache::~sprite_info_cache()
{
}

sprite_info_set& sprite_info_cache::operator[](const std::string& filename)
{
    return (*pimpl_)[filename];
}

void sprite_info_cache::clear()
{
    pimpl_->clear();
}
