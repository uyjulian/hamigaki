// sprite_info.hpp: sprite infomation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef SPRITE_INFO_HPP
#define SPRITE_INFO_HPP

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

    static const std::size_t nform = static_cast<std::size_t>(-1);

    std::string texture() const { return texture_; }
    void texture(const std::string& filename) { texture_ = filename; }

    int width() const { return width_; }
    void width(int w) { width_ = w; }

    int height() const { return height_; }
    void height(int h) { height_ = h; }

    void push_back(std::size_t no, const sprite_info& info)
    {
        if (no >= list_.size())
            list_.resize(no+1);

        group_type& group = list_[no];
        group.push_back(info);
    }

    std::size_t size() const
    {
        return list_.size();
    }

    const group_type& get_group(std::size_t no) const
    {
        return list_[no];
    }

    void swap(sprite_info_set& rhs)
    {
        texture_.swap(rhs.texture_);
        std::swap(width_, rhs.width_);
        std::swap(height_, rhs.height_);
        list_.swap(rhs.list_);
    }

private:
    std::string texture_;
    int width_;
    int height_;
    std::vector<group_type> list_;
};

void
load_sprite_info_set_from_text(sprite_info_set& infos, const char* filename);

#endif // SPRITE_INFO_HPP
