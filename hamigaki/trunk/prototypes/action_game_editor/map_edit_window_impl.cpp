// map_edit_window_impl.cpp: the window implementation for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "map_edit_window_impl.hpp"
#include "direct3d9.hpp"
#include "direct3d_device9.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"
#include "stage_map.hpp"
#include <boost/optional.hpp>
#include <algorithm>
#include <utility>

class map_edit_window::impl
{
private:
    typedef std::pair<int,int> texture_pos;

public:
    explicit impl(::HWND handle)
        : handle_(handle), selected_char_(' ')
    {
    }

    ~impl()
    {
    }

    void load_stage(const std::string& filename)
    {
        load_map_from_text(filename.c_str(), map_);
        stage_file_ = filename;

        int vert_max = update_scroll_box().second;

        ::SetScrollPos(handle_, SB_HORZ, 0, TRUE);
        ::SetScrollPos(handle_, SB_VERT, vert_max, TRUE);

        ::InvalidateRect(handle_, 0, FALSE);
    }

    void render()
    {
        if (!device_)
            connect_d3d_device();

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        int min_x = horz_scroll_pos();
        int min_y = vert_scroll_pos();

        int max_x = min_x + (cr.right - cr.left + 31) / 32;
        int max_y = min_y + (cr.bottom - cr.top + 31) / 32;

        device_.clear_target(D3DCOLOR_XRGB(0x77,0x66,0xDD));
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            device_.set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            for (int y = min_y; y < max_y; ++y)
            {
                for (int x = min_x; x < max_x; ++x)
                {
                    char type = map_(x, y);
                    boost::optional<texture_pos> pos = get_texture_pos(type);
                    if (pos)
                        draw_character(x-min_x, y-min_y, *pos);
                }
            }

            draw_sprite(
                device_,
                static_cast<float>(cursor_pos_.first*32),
                cr.bottom - static_cast<float>((cursor_pos_.second+1)*32),
                0.0f, cursor_texture_
            );

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
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        ::SCROLLINFO old = {};
        old.cbSize = sizeof(old);
        old.fMask = SIF_POS | SIF_RANGE;
        ::GetScrollInfo(handle_, SB_VERT, &old);

        int width = cr.right - cr.left;
        int horz_max = (std::max)(map_.width() - width/32, 0);
        ::SetScrollRange(handle_, SB_HORZ, 0, horz_max, TRUE);

        int height = cr.bottom - cr.top;
        int vert_max = (std::max)(map_.height() - height/32, 0);
        ::SetScrollRange(handle_, SB_VERT, 0, vert_max, TRUE);

        int vert = (std::max)(vert_max-(old.nMax-old.nPos), 0);
        ::SetScrollPos(handle_, SB_VERT, vert, TRUE);

        return std::make_pair(horz_max, vert_max);
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
        cursor_pos_.first = x;
        cursor_pos_.second = y;

        ::InvalidateRect(handle_, 0, FALSE);
    }

    void select_char(char c)
    {
        selected_char_ = c;
    }

    void put_char()
    {
        int x = horz_scroll_pos() + cursor_pos_.first;
        int y = vert_scroll_pos() + cursor_pos_.second;
        map_.replace(x, y, selected_char_);

        ::InvalidateRect(handle_, 0, FALSE);
    }

private:
    ::HWND handle_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    std::string stage_file_;
    stage_map map_;
    direct3d_texture9 chips_texture_;
    direct3d_texture9 cursor_texture_;
    std::pair<int,int> cursor_pos_;
    char selected_char_;

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

        chips_texture_ = create_png_texture(device_, "char_chips.png");
        cursor_texture_ = create_png_texture(device_, "box_cursor.png");
    }

    boost::optional<texture_pos> get_texture_pos(char type)
    {
        if (type == '@')
            return texture_pos(32*1, 32*0);
        else if (type == 'U')
            return texture_pos(32*2, 32*0);
        else if (type == 'D')
            return texture_pos(32*3, 32*0);
        else if (type == 'o')
            return texture_pos(32*0, 32*1);
        else if (type == 'a')
            return texture_pos(32*1, 32*1);
        else if (type == 'p')
            return texture_pos(32*2, 32*1);
        else if (type == 'w')
            return texture_pos(32*3, 32*1);
        else if (type == '=')
            return texture_pos(32*0, 32*2);
        else if (type == 'G')
            return texture_pos(32*1, 32*2);
        else if (type == 'I')
            return texture_pos(32*2, 32*2);
        else if (type == 'm')
            return texture_pos(32*3, 32*2);
        else if (type == '$')
            return texture_pos(32*0, 32*3);
        else if (type == '?')
            return texture_pos(32*1, 32*3);
        else if (type == 'S')
            return texture_pos(32*2, 32*3);
        else if (type == '/')
            return texture_pos(32*0, 32*4);
        else if (type == '\\')
            return texture_pos(32*1, 32*4);
        else
            return boost::optional<texture_pos>();
    }

    void draw_character(int x, int y, const texture_pos& pos)
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        float left = static_cast<float>(x * 32);
        float bottom = static_cast<float>((cr.bottom - cr.top) - y * 32);
        float top = bottom - 32.0f;

        ::draw_sprite(
            device_, left, top, 0.0f, chips_texture_,
            pos.first, pos.second, 32, 32, false
        );
    }

    int horz_scroll_pos() const
    {
        return ::GetScrollPos(handle_, SB_HORZ);
    }

    int vert_scroll_pos() const
    {
        ::SCROLLINFO info = {};
        info.cbSize = sizeof(info);
        info.fMask = SIF_POS | SIF_RANGE;
        ::GetScrollInfo(handle_, SB_VERT, &info);
        return info.nMax - info.nPos;
    }
};

map_edit_window::map_edit_window(::HWND handle) : pimpl_(new impl(handle))
{
}

map_edit_window::~map_edit_window()
{
}

void map_edit_window::load_stage(const std::string& filename)
{
    pimpl_->load_stage(filename);
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

void map_edit_window::select_char(char c)
{
    pimpl_->select_char(c);
}

void map_edit_window::put_char()
{
    pimpl_->put_char();
}
