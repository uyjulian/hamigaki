// sprite_info.hpp: sprite infomation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SPRITE_INFO_HPP
#define SPRITE_INFO_HPP

#include <boost/cstdint.hpp>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

struct sprite_info
{
    int x;
    int y;

    int left;
    int top;
    int width;
    int height;
};

class sprite_info_set
{
public:
    typedef std::vector<sprite_info> group_type;
    typedef std::map<boost::uint32_t,group_type> table_type;

    static const boost::uint32_t nform = static_cast<boost::uint32_t>(-1);

    std::string texture() const { return texture_; }
    void texture(const std::string& filename) { texture_ = filename; }

    int width() const { return width_; }
    void width(int w) { width_ = w; }

    int height() const { return height_; }
    void height(int h) { height_ = h; }

    void push_back(boost::uint32_t form, const sprite_info& info)
    {
        group_type& group = table_[form];
        group.push_back(info);
    }

    const group_type& get_group(boost::uint32_t form) const
    {
        table_type::const_iterator pos = table_.find(form);
        if (pos == table_.end())
            throw std::runtime_error("cannot find sprite group");
        return pos->second;
    }

    void swap(sprite_info_set& rhs)
    {
        texture_.swap(rhs.texture_);
        std::swap(width_, rhs.width_);
        std::swap(height_, rhs.height_);
        table_.swap(rhs.table_);
    }

private:
    std::string texture_;
    int width_;
    int height_;
    std::map<boost::uint32_t,group_type> table_;
};

void
load_sprite_info_set_from_text(const char* filename, sprite_info_set& infos);

#endif // SPRITE_INFO_HPP
