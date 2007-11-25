// map_edit_window_impl.cpp: the window implementation for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "map_edit_window_impl.hpp"
#include "direct3d9.hpp"
#include "direct3d_device9.hpp"
#include "sprite.hpp"
#include "sprite_info_cache.hpp"
#include "stage_map.hpp"
#include "texture_cache.hpp"
#include <algorithm>
#include <utility>

namespace
{

inline bool is_enemy(char type)
{
    switch (type)
    {
        case 'o': case 'a': case 'p': case 'w':
            return true;
        default:
            return false;
    }
}

unsigned long sprite_color(char type)
{
    if (type == 'S')
        return 0x80FFFFFFul;
    else
        return 0xFFFFFFFFul;
}

} // namespace

class map_edit_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle), textures_(device_)
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

        int min_x = ::GetScrollPos(handle_, SB_HORZ);

        ::SCROLLINFO info = {};
        info.cbSize = sizeof(info);
        info.fMask = SIF_POS | SIF_RANGE;
        ::GetScrollInfo(handle_, SB_VERT, &info);
        int min_y = info.nMax - info.nPos;

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
                    if (const sprite_info_set* p = get_sprite_info(type))
                    {
                        bool back = is_enemy(type);
                        unsigned long color = sprite_color(type);
                        draw_character(x-min_x, y-min_y, *p, back, color);
                    }
                }
            }

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

private:
    ::HWND handle_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    std::string stage_file_;
    stage_map map_;
    sprite_info_cache sprites_;
    texture_cache textures_;

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
    }

    const sprite_info_set* get_sprite_info(char type)
    {
        if (type == '@')
            return &sprites_["boy.txt"];
        else if (type == 'o')
            return &sprites_["ball.txt"];
        else if (type == 'a')
            return &sprites_["ball.txt"];
        else if (type == 'p')
            return &sprites_["ball.txt"];
        else if (type == 'w')
            return &sprites_["ball.txt"];
        else if (type == 'U')
            return &sprites_["lift.txt"];
        else if (type == 'D')
            return &sprites_["lift.txt"];
        else if (type == '=')
            return &sprites_["brick_block.txt"];
        else if (type == 'G')
            return &sprites_["brick_block.txt"];
        else if (type == 'I')
            return &sprites_["brick_block.txt"];
        else if (type == 'm')
            return &sprites_["used_block.txt"];
        else if (type == '$')
            return &sprites_["item_box.txt"];
        else if (type == '?')
            return &sprites_["item_box.txt"];
        else if (type == 'S')
            return &sprites_["used_block.txt"];
        else if (type == '/')
            return &sprites_["left_down.txt"];
        else if (type == '\\')
            return &sprites_["right_down.txt"];
        else
            return 0;
    }

    void draw_character(
        int x, int y, const sprite_info_set& infos,
        bool back, unsigned long color)
    {
        const std::vector<sprite_info>& group =
            infos.get_group(sprite_form::normal);

        const sprite_info& info = group[0];

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        float half_width = static_cast<float>(infos.width()) * 0.5f;
        float left = static_cast<float>(x * 32 + 16) - half_width;
        float bottom = static_cast<float>((cr.bottom - cr.top) - y * 32);
        float top = bottom - static_cast<float>(infos.height());

        const std::string& texture = infos.texture();
        if (!texture.empty())
        {
            ::draw_sprite(
                device_, left, top, 0.0f, textures_[texture],
                info.x, info.y, infos.width(), infos.height(), back, color
            );
        }
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
