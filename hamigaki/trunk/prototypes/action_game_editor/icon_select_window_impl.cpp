// icon_select_window_impl.cpp: the window implementation for icon select

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "icon_select_window_impl.hpp"
#include "direct3d9.hpp"
#include "direct3d_device9.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"

class icon_select_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle)
    {
    }

    ~impl()
    {
    }

    void render()
    {
        if (!device_)
            connect_d3d_device();

        device_.clear_target(D3DCOLOR_XRGB(0x77,0x66,0xDD));
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            device_.set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            if (chips_texture_)
                draw_sprite(device_, 0.0f, 0.0f, 0.0f, chips_texture_);

            draw_sprite(
                device_,
                static_cast<float>(cursor_pos_.first*32),
                static_cast<float>(cursor_pos_.second*32),
                0.0f, cursor_texture_
            );

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);
        }
        device_.present();
    }

    void cursor_pos(int x, int y)
    {
        cursor_pos_.first = x;
        cursor_pos_.second = y;

        ::InvalidateRect(handle_, 0, FALSE);
    }

    std::pair<int,int> cursor_pos() const
    {
        return cursor_pos_;
    }

    void load(const std::string& filename)
    {
        if (!device_)
            connect_d3d_device();

        chips_texture_ = create_png_texture(device_, filename);
        cursor_pos(0, 0);
    }

private:
    ::HWND handle_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    direct3d_texture9 chips_texture_;
    direct3d_texture9 cursor_texture_;
    std::pair<int,int> cursor_pos_;

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

        cursor_texture_ = create_png_texture(device_, "box_cursor.png");
    }
};

icon_select_window::icon_select_window(::HWND handle) : pimpl_(new impl(handle))
{
}

icon_select_window::~icon_select_window()
{
}

void icon_select_window::render()
{
    pimpl_->render();
}

void icon_select_window::cursor_pos(int x, int y)
{
    pimpl_->cursor_pos(x, y);
}

std::pair<int,int> icon_select_window::cursor_pos() const
{
    return pimpl_->cursor_pos();
}

void icon_select_window::load(const std::string& filename)
{
    pimpl_->load(filename);
}
