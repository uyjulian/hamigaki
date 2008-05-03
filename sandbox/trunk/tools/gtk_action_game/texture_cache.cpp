// texture_cache.cpp: the cache for textures

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "texture_cache.hpp"
#include "png_reader.hpp"
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>
#include <fstream>

namespace
{

void load_png_to_texture(hamigaki::texture& tex, const std::string& filename)
{
    std::ifstream is(filename.c_str(), std::ios_base::binary);

    hamigaki::png_reader png(is);
    const int width = static_cast<int>(png.width());
    const int height = static_cast<int>(png.height());
    const unsigned long pitch = png.pitch();

    typedef unsigned char ubyte;
    boost::scoped_array<ubyte> row(new ubyte[pitch]);
    boost::scoped_array<ubyte> data(new ubyte[4*width*height]);

    const std::size_t data_pitch = static_cast<std::size_t>(4*width);
    for (int i = 0; i < height; ++i)
    {
        png.read_next_row(row.get());
        std::memcpy(&data[data_pitch*i], row.get(), data_pitch);
    }

    tex.set_image(width, height, data.get());
}

} // namespace

class texture_cache::impl
{
private:
    typedef boost::ptr_map<std::string,hamigaki::texture> table_type;

public:
    explicit impl(hamigaki::render_context& rc) : rc_(rc)
    {
    }

    ~impl()
    {
    }

    hamigaki::texture& operator[](const std::string& filename)
    {
        table_type::iterator pos = table_.lower_bound(filename);
        if ((pos == table_.end()) || (pos->first > filename))
        {
            std::auto_ptr<hamigaki::texture> tmp(new hamigaki::texture(rc_));
            load_png_to_texture(*tmp, filename);
            pos = table_.insert(filename, tmp).first;
        }
        return *pos->second;
    }

    void clear()
    {
        table_.clear();
    }

private:
    hamigaki::render_context& rc_;
    table_type table_;
};

texture_cache::texture_cache(hamigaki::render_context& rc)
    : pimpl_(new impl(rc))
{
}

texture_cache::~texture_cache()
{
}

hamigaki::texture& texture_cache::operator[](const std::string& filename)
{
    return (*pimpl_)[filename];
}

void texture_cache::clear()
{
    pimpl_->clear();
}
