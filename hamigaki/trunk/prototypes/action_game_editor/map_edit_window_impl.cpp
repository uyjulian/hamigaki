// map_edit_window_impl.cpp: the window implementation for stage map

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "map_edit_window_impl.hpp"
#include "cursor.hpp"
#include "direct3d9.hpp"
#include "direct3d_device9.hpp"
#include "draw.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"
#include "sprite_info_cache.hpp"
#include "stage_map_load.hpp"
#include "stage_map_save.hpp"
#include "texture_cache.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <utility>

class map_edit_window::impl
{
private:
    typedef std::pair<int,int> texture_pos;

public:
    explicit impl(::HWND handle)
        : handle_(handle), bg_color_(0), map_(0), textures_(device_), chars_(0)
        , mouse_captured_(false)
    {
    }

    ~impl()
    {
    }

    void set_bg_color(unsigned long color)
    {
        bg_color_ = color;
        ::InvalidateRect(handle_, 0, FALSE);
    }

    void set_characters(std::set<game_character_class>* chars)
    {
        if (!device_)
            connect_d3d_device();

        chars_ = chars;
        textures_.clear();
        sprites_.clear();

        if (chars_)
        {
            typedef std::set<game_character_class>::iterator iter_type;
            for (iter_type i=chars_->begin(), end=chars_->end(); i!=end; ++i)
            {
                if (i->icon.empty())
                {
                    sprite_info_set& infos = sprites_[i->sprite];
                    textures_[infos.texture];
                }
                else
                    textures_[i->icon];
            }
        }
    }

    void set_stage(stage_map* map)
    {
        map_ = map;

        int vert_max = update_scroll_box().second;

        ::SetScrollPos(handle_, SB_HORZ, 0, TRUE);
        ::SetScrollPos(handle_, SB_VERT, vert_max, TRUE);

        ::InvalidateRect(handle_, 0, FALSE);
    }

    void render()
    {
        if (!device_)
            connect_d3d_device();

        if (!map_)
        {
            device_.clear_target(0xFFC0C0C0ul);
            device_.present();
            return;
        }

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        int min_x = horz_scroll_value()*16 + 16;
        int min_y = vert_scroll_value()*16;

        int max_x = min_x + cr.right;
        int max_y = min_y + cr.bottom;

        typedef map_elements::const_iterator iter_type;
        const map_elements& x_y = map_->elements;
        iter_type beg = x_y.lower_bound(std::make_pair(min_x-16, 0));
        iter_type end = x_y.upper_bound(std::make_pair(max_x, map_->height));

        int cursor_x = min_x + cursor_pos_.first*16;
        int cursor_y = min_y + cursor_pos_.second*16;

        device_.clear_target(bg_color_);
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            device_.set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            for ( ; beg != end; ++beg)
            {
                int x = beg->first.first;
                int y = beg->first.second;
                const hamigaki::uuid& type = beg->second;

                draw_character((x-min_x)/16, (y-min_y)/16, type);
            }

            if (max_x > map_->width)
            {
                ::draw_rectangle(
                    device_,
                    map_->width-min_x+16.0f, 0.0f, 0.0f,
                    max_x-static_cast<float>(map_->width),
                    static_cast<float>(cr.bottom),
                    0xFFC0C0C0ul
                );
            }

            if (max_y > map_->height)
            {
                ::draw_rectangle(
                    device_,
                    0.0f, 0.0f, 0.0f,
                    static_cast<float>(cr.right),
                    max_y-static_cast<float>(map_->height),
                    0xFFC0C0C0ul
                );
            }

            if ((cursor_x < map_->width) && (cursor_y < map_->height))
                draw_cursor();

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);
        }
        device_.present();
    }

    void reset_d3d()
    {
        if (!device_)
            return;

        ::D3DPRESENT_PARAMETERS params; 
        std::memset(&params, 0, sizeof(params));
        params.Windowed = TRUE;
        params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        params.BackBufferFormat = D3DFMT_UNKNOWN;

        device_.reset(params);
    }

    std::pair<int,int> update_scroll_box()
    {
        int map_width = map_ ? map_->width : 0;
        int map_height = map_ ? map_->height : 0;

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        int old_vert = vert_scroll_value();

        ::SCROLLINFO horz = {};
        horz.cbSize = sizeof(horz);
        horz.fMask = SIF_RANGE | SIF_PAGE;
        horz.nMin = 0;
        horz.nMax = map_width/16 - 1;
        horz.nPage = cr.right / 16;
        ::SetScrollInfo(handle_, SB_HORZ, &horz, TRUE);

        ::SCROLLINFO vert = {};
        vert.cbSize = sizeof(vert);
        vert.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        vert.nMin = 0;
        vert.nMax = map_height/16 - 1;
        vert.nPage = cr.bottom / 16;
        vert.nPos = vert.nMax - old_vert - static_cast<int>(vert.nPage) + 1;
        ::SetScrollInfo(handle_, SB_VERT, &vert, TRUE);

        int max_horz_pos = horz.nMax - horz.nPage;
        if (max_horz_pos < 0)
            max_horz_pos = 0;

        int max_vert_pos = vert.nMax - vert.nPage;
        if (max_vert_pos < 0)
            max_vert_pos = 0;

        return std::make_pair(max_horz_pos, max_vert_pos);
    }

    void horz_scroll_pos(int pos)
    {
        ::SetScrollPos(handle_, SB_HORZ, pos, TRUE);
        ::InvalidateRect(handle_, 0, FALSE);
    }

    void vert_scroll_pos(int pos)
    {
        ::SetScrollPos(handle_, SB_VERT, pos, TRUE);
        ::InvalidateRect(handle_, 0, FALSE);
    }

    void cursor_pos(int x, int y)
    {
        std::pair<int,int> pos(x, y);

        if (pos != cursor_pos_)
        {
            cursor_pos_ = pos;
            ::InvalidateRect(handle_, 0, FALSE);
        }
    }

    void select_char(const hamigaki::uuid& c)
    {
        selected_char_ = c;
    }

    void put_char()
    {
        if (!map_)
            return;

        int x = (horz_scroll_value() + cursor_pos_.first) * 16 + 16;
        int y = (vert_scroll_value() + cursor_pos_.second) * 16;

        if ((x >= map_->width) || (y >= map_->height))
            return;

        typedef map_elements::iterator iter_type;
        map_elements& x_y = map_->elements;

        if (!selected_char_.is_null())
        {
            std::pair<int,int> pos(x, y);
            iter_type beg = x_y.lower_bound(std::make_pair(x-31, y-31));
            iter_type end = x_y.lower_bound(std::make_pair(x+31, y+32));

            for ( ; beg != end; ++beg)
            {
                const std::pair<int,int>& pos2 = beg->first;
                if (pos2 == pos)
                    break;
                else if ((pos2.second > y-32) && (pos2.second < y+32))
                    return;
            }
        }

        iter_type old = x_y.find(std::make_pair(x, y));
        if (old != x_y.end())
        {
            if (selected_char_.is_null())
                x_y.erase(old);
            else if (old->second == selected_char_)
                return;
            else
                old->second = selected_char_;
        }
        else
        {
            if (selected_char_.is_null())
                return;
            else
                x_y[std::make_pair(x, y)] = selected_char_;
        }

        map_->modified = true;
        ::InvalidateRect(handle_, 0, FALSE);
    }

    bool modified() const
    {
        return (map_ != 0) && map_->modified;
    }

    void mouse_captured(bool value)
    {
        mouse_captured_ = value;
    }

    bool mouse_captured() const
    {
        return mouse_captured_;
    }

private:
    ::HWND handle_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    unsigned long bg_color_;
    stage_map* map_;
    direct3d_texture9 cursor_texture_;
    texture_cache textures_;
    std::set<game_character_class>* chars_;
    sprite_info_cache sprites_;
    std::pair<int,int> cursor_pos_;
    hamigaki::uuid selected_char_;
    bool mouse_captured_;

    void connect_d3d_device()
    {
        ::D3DPRESENT_PARAMETERS params; 
        std::memset(&params, 0, sizeof(params));
        params.Windowed = TRUE;
        params.SwapEffect = D3DSWAPEFFECT_DISCARD;
        params.BackBufferFormat = D3DFMT_UNKNOWN;

        device_ = d3d_.create_device(
            D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle_,
            D3DCREATE_HARDWARE_VERTEXPROCESSING, params);

        cursor_texture_ = create_cursor_texture(device_, 32, 32);
    }

    void draw_box(int x, int y, unsigned long color)
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        float left = static_cast<float>(x * 16);
        float bottom = static_cast<float>(cr.bottom - y * 16);
        float top = bottom - 32.0f;

        ::draw_rectangle(device_, left, top, 0.0f, 32.0f, 32.0f, color);
    }

    void draw_character(int x, int y, const hamigaki::uuid& type)
    {
        typedef std::set<game_character_class>::iterator iter_type;

        if (chars_ == 0)
            return;

        game_character_class dummy;
        dummy.id = type;
        iter_type pos = chars_->find(dummy);
        if (pos == chars_->end())
            return;

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        float left = static_cast<float>(x * 16);
        float bottom = static_cast<float>(cr.bottom - y * 16);
        float top = bottom - 32.0f;

        if (pos->icon.empty())
        {
            sprite_info_set& infos = sprites_[pos->sprite];
            const sprite_pattern& pattern =
                infos.groups[sprite_form::normal].patterns.at(0);

            draw_sprite(
                device_,
                left, top, 0.0f, 32.0f, 32.0f,
                textures_[infos.texture],
                infos.width * pattern.x,
                infos.height * pattern.y,
                infos.width,
                infos.height, 0
            );
        }
        else
        {
            const rectangle<int>& r = pos->icon_rect;

            draw_sprite(
                device_,
                left, top, 0.0f, 32.0f, 32.0f,
                textures_[pos->icon],
                r.x, r.y, r.lx, r.ly, 0
            );
        }
    }

    void draw_cursor()
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        draw_sprite(
            device_,
            static_cast<float>(cursor_pos_.first*16),
            cr.bottom - static_cast<float>(cursor_pos_.second*16+32),
            0.0f, cursor_texture_
        );
    }

    int horz_scroll_value() const
    {
        return ::GetScrollPos(handle_, SB_HORZ);
    }

    int vert_scroll_value() const
    {
        ::SCROLLINFO info = {};
        info.cbSize = sizeof(info);
        info.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        ::GetScrollInfo(handle_, SB_VERT, &info);

        int value = info.nMax - info.nPos - static_cast<int>(info.nPage) + 1;
        return (std::max)(value, 0);
    }
};

map_edit_window::map_edit_window(::HWND handle) : pimpl_(new impl(handle))
{
}

map_edit_window::~map_edit_window()
{
}

void map_edit_window::set_bg_color(unsigned long color)
{
    pimpl_->set_bg_color(color);
}

void map_edit_window::set_characters(std::set<game_character_class>* chars)
{
    pimpl_->set_characters(chars);
}

void map_edit_window::set_stage(stage_map* map)
{
    pimpl_->set_stage(map);
}

void map_edit_window::render()
{
    pimpl_->render();
}

void map_edit_window::reset_d3d()
{
    pimpl_->reset_d3d();
}

void map_edit_window::update_scroll_box()
{
    pimpl_->update_scroll_box();
}

void map_edit_window::horz_scroll_pos(int pos)
{
    pimpl_->horz_scroll_pos(pos);
}

void map_edit_window::vert_scroll_pos(int pos)
{
    pimpl_->vert_scroll_pos(pos);
}

void map_edit_window::cursor_pos(int x, int y)
{
    pimpl_->cursor_pos(x, y);
}

void map_edit_window::select_char(const hamigaki::uuid& c)
{
    pimpl_->select_char(c);
}

void map_edit_window::put_char()
{
    pimpl_->put_char();
}

bool map_edit_window::modified() const
{
    return pimpl_->modified();
}

void map_edit_window::mouse_captured(bool value)
{
    pimpl_->mouse_captured(value);
}

bool map_edit_window::mouse_captured() const
{
    return pimpl_->mouse_captured();
}
