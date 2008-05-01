// texture_cache.cpp: the cache for textures

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "texture_cache.hpp"
#include "direct3d_texture9.hpp"
#include "png_loader.hpp"
#include <map>

class texture_cache::impl
{
private:
    typedef std::map<std::string,direct3d_texture9> table_type;

public:
    explicit impl(direct3d_device9& device) : device_(device)
    {
    }

    ~impl()
    {
    }

    direct3d_texture9& operator[](const std::string& filename)
    {
        table_type::iterator pos = table_.lower_bound(filename);
        if ((pos == table_.end()) || (pos->first > filename))
        {
            const direct3d_texture9& t = create_png_texture(device_, filename);
            pos = table_.insert(pos, table_type::value_type(filename, t));
        }
        return pos->second;
    }

    void clear()
    {
        table_.clear();
    }

private:
    direct3d_device9& device_;
    std::map<std::string,direct3d_texture9> table_;
};

texture_cache::texture_cache(direct3d_device9& device)
    : pimpl_(new impl(device))
{
}

texture_cache::~texture_cache()
{
}

direct3d_texture9& texture_cache::operator[](const std::string& filename)
{
    return (*pimpl_)[filename];
}

void texture_cache::clear()
{
    pimpl_->clear();
}
