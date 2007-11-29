// map_edit_window_impl.cpp: the window implementation for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "map_edit_window_impl.hpp"
#include "direct3d9.hpp"
#include "direct3d_device9.hpp"
#include "draw.hpp"
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
        , modified_(false), mouse_captured_(false)
    {
    }

    ~impl()
    {
    }

    void new_stage(int width, int height)
    {
        stage_map tmp;
        std::string line(static_cast<std::string::size_type>(width), ' ');
        for (int i = 0; i < height; ++i)
            tmp.push_back(line);

        map_.swap(tmp);
        modified_ = true;

        int vert_max = update_scroll_box().second;

        ::SetScrollPos(handle_, SB_HORZ, 0, TRUE);
        ::SetScrollPos(handle_, SB_VERT, vert_max, TRUE);

        ::InvalidateRect(handle_, 0, FALSE);
    }

    void load_stage(const std::string& filename)
    {
        load_map_from_text(filename.c_str(), map_);
        modified_ = false;

        int vert_max = update_scroll_box().second;

        ::SetScrollPos(handle_, SB_HORZ, 0, TRUE);
        ::SetScrollPos(handle_, SB_VERT, vert_max, TRUE);

        ::InvalidateRect(handle_, 0, FALSE);
    }

    void save_stage(const std::string& filename)
    {
        save_map_to_text(filename.c_str(), map_);
        modified_ = false;
    }

    void render()
    {
        if (!device_)
            connect_d3d_device();

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        int min_x = horz_scroll_value();
        int min_y = vert_scroll_value();

        int max_x = min_x + (cr.right - cr.left + 31) / 32;
        int max_y = min_y + (cr.bottom - cr.top + 31) / 32;

        int cursor_x = min_x + cursor_pos_.first;
        int cursor_y = min_y + cursor_pos_.second;

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
                    if ((x < map_.width()) && (y < map_.height()))
                    {
                        char type = map_(x, y);
                        boost::optional<texture_pos> pos
                            = get_texture_pos(type);

                        if (pos)
                            draw_character(x-min_x, y-min_y, *pos);
                    }
                    else
                        draw_box(x-min_x, y-min_y, 0xFFC0C0C0ul);
                }
            }

            if ((cursor_x < map_.width()) && (cursor_y < map_.height()))
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
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        int old_vert = vert_scroll_value();

        ::SCROLLINFO horz = {};
        horz.cbSize = sizeof(horz);
        horz.fMask = SIF_RANGE | SIF_PAGE;
        horz.nMin = 0;
        horz.nMax = map_.width() - 1;
        horz.nPage = cr.right / 32;
        ::SetScrollInfo(handle_, SB_HORZ, &horz, TRUE);

        ::SCROLLINFO vert = {};
        vert.cbSize = sizeof(vert);
        vert.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        vert.nMin = 0;
        vert.nMax = map_.height() - 1;
        vert.nPage = cr.bottom / 32;
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

    void select_char(char c)
    {
        selected_char_ = c;
    }

    void put_char()
    {
        int x = horz_scroll_value() + cursor_pos_.first;
        int y = vert_scroll_value() + cursor_pos_.second;

        if (map_(x, y) != selected_char_)
        {
            map_.replace(x, y, selected_char_);
            modified_ = true;
            ::InvalidateRect(handle_, 0, FALSE);
        }
    }

    bool modified() const
    {
        return modified_;
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
    stage_map map_;
    direct3d_texture9 chips_texture_;
    direct3d_texture9 cursor_texture_;
    std::pair<int,int> cursor_pos_;
    char selected_char_;
    bool modified_;
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

    void draw_box(int x, int y, unsigned long color)
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        float left = static_cast<float>(x * 32);
        float bottom = static_cast<float>((cr.bottom - cr.top) - y * 32);
        float top = bottom - 32.0f;

        ::draw_rectangle(device_, left, top, 0.0f, 32.0f, 32.0f, color);
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

    void draw_cursor()
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        draw_sprite(
            device_,
            static_cast<float>(cursor_pos_.first*32),
            cr.bottom - static_cast<float>((cursor_pos_.second+1)*32),
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

void map_edit_window::new_stage(int width, int height)
{
    pimpl_->new_stage(width, height);
}

void map_edit_window::load_stage(const std::string& filename)
{
    pimpl_->load_stage(filename);
}

void map_edit_window::save_stage(const std::string& filename)
{
    pimpl_->save_stage(filename);
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
