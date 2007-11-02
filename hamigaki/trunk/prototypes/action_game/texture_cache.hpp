// texture_cache.hpp: the cache for textures

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef TEXTURE_CACHE_HPP
#define TEXTURE_CACHE_HPP

#include <boost/shared_ptr.hpp>
#include <string>

class direct3d_device9;
class direct3d_texture9;

class texture_cache
{
public:
    explicit texture_cache(direct3d_device9& device);
    ~texture_cache();
    direct3d_texture9& operator[](const std::string& filename);
    void clear();

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

#endif // TEXTURE_CACHE_HPP
