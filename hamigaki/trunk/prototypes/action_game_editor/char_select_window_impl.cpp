// char_select_window_impl.cpp: the window implementation for character select

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "char_select_window_impl.hpp"
#include "char_select_window_msgs.hpp"
#include "cursor.hpp"
#include "direct3d9.hpp"
#include "direct3d_device9.hpp"
#include "msg_utilities.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"

namespace
{

const ::GUID guids[] =
{
    { 0xD5D26CC5, 0xD8BD, 0x40A4, {0x83,0x9E,0xE3,0xC7,0xFA,0x57,0x0F,0x66} },
    { 0xF19E0B84, 0x45A2, 0x4FC6, {0x96,0x8A,0xE4,0x49,0x80,0x9A,0xC6,0x92} },
    { 0xB2107203, 0x173F, 0x4AAA, {0x92,0x28,0xE9,0x54,0xED,0xBF,0x9C,0x2F} },
    { 0x6F642A22, 0x86B5, 0x41BF, {0x99,0x03,0x44,0xC0,0x0B,0x45,0x4D,0x6C} },
    { 0x1831D6D2, 0x0A6A, 0x4B73, {0x9F,0x92,0xE9,0x4B,0x56,0xEC,0xB9,0x70} },
    { 0x52B81339, 0x4AA0, 0x47D4, {0x9C,0xC8,0xE7,0x7A,0x02,0x3E,0xCA,0x7F} },
    { 0x5D55A5B6, 0x66B1, 0x4D4A, {0xAB,0xB4,0xAB,0xE3,0x38,0x94,0x6D,0x28} },
    { 0x8DC4CEDB, 0xDF9F, 0x4B9E, {0xB2,0xC3,0x0D,0x4A,0x6D,0x89,0x8D,0xF3} },
    { 0x9F211EEC, 0xAA1B, 0x4704, {0xAA,0x25,0x5C,0x40,0x19,0x24,0xDD,0xAD} },
    { 0xF311F6F5, 0x8B30, 0x48CE, {0xA8,0x43,0x10,0xDA,0xC4,0x26,0x84,0xDA} },
    { 0x7FB448BA, 0xDAE6, 0x45CB, {0x9E,0xD1,0xC1,0x6A,0x7C,0xFB,0x0C,0x97} },
    { 0x2FC40FC2, 0x841F, 0x48BC, {0xB9,0x10,0xFD,0x03,0xA6,0x13,0xE5,0x8A} },
    { 0x106AD0B7, 0x9A0A, 0x4351, {0xAD,0xB4,0xE2,0x1A,0x4C,0xFC,0xEC,0xD5} },
    { 0x2F1830C6, 0x6734, 0x4D14, {0x9A,0x15,0x6A,0xD5,0xD6,0x6D,0x4B,0xFA} },
    { 0x1047BBAC, 0x5FC8, 0x4435, {0xBD,0x26,0xAA,0x0A,0x11,0x87,0xBE,0x99} },
    { 0x639CE8F6, 0xDB5A, 0x45DA, {0xA9,0x63,0x9D,0x93,0xF7,0x11,0xD3,0xFE} }
};

} // namespace

class char_select_window::impl
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

        int code = char_select_window_msgs::notify_sel_changed;
        int id = ::GetDlgCtrlID(handle_);

        send_command(::GetParent(handle_), id, code, handle_);
    }

    hamigaki::uuid selected_char() const
    {
        std::size_t index =
            static_cast<std::size_t>(cursor_pos_.first + cursor_pos_.second*4);

        if ((index != 0) && (--index < sizeof(guids)/sizeof(guids[0])))
            return hamigaki::uuid(guids[index]);
        else
            return hamigaki::uuid();
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

        chips_texture_ = create_png_texture(device_, "char_chips.png");
        cursor_texture_ = create_cursor_texture(device_, 32, 32);
    }
};

char_select_window::char_select_window(::HWND handle) : pimpl_(new impl(handle))
{
}

char_select_window::~char_select_window()
{
}

void char_select_window::render()
{
    pimpl_->render();
}

void char_select_window::cursor_pos(int x, int y)
{
    pimpl_->cursor_pos(x, y);
}

hamigaki::uuid char_select_window::selected_char() const
{
    return pimpl_->selected_char();
}
