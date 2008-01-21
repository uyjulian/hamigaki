// icon_view_window_impl.cpp: the window implementation for icon view

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "icon_view_window_impl.hpp"
#include "direct3d9.hpp"
#include "direct3d_device9.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"

class icon_view_window::impl
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
            {
                draw_sprite(
                    device_, 0.0f, 0.0f, 0.0f, 32.0f, 32.0f,
                    chips_texture_, rect_.x, rect_.y, rect_.lx, rect_.ly, 0
                );
            }

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);
        }
        device_.present();
    }

    void load(const std::string& filename, const rectangle<int>& r)
    {
        if (!device_)
            connect_d3d_device();

        chips_texture_ = create_png_texture(device_, filename);
        filename_ = filename;
        rect_ = r;

        ::InvalidateRect(handle_, 0, FALSE);
    }

    std::string filename() const
    {
        return filename_;
    }

    rectangle<int> icon_rect() const
    {
        return rect_;
    }

private:
    ::HWND handle_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    direct3d_texture9 chips_texture_;
    std::string filename_;
    rectangle<int> rect_;

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
};

icon_view_window::icon_view_window(::HWND handle) : pimpl_(new impl(handle))
{
}

icon_view_window::~icon_view_window()
{
}

void icon_view_window::render()
{
    pimpl_->render();
}

void
icon_view_window::load(const std::string& filename, const rectangle<int>& r)
{
    pimpl_->load(filename, r);
}

std::string icon_view_window::filename() const
{
    return pimpl_->filename();
}

rectangle<int> icon_view_window::icon_rect() const
{
    return pimpl_->icon_rect();
}
