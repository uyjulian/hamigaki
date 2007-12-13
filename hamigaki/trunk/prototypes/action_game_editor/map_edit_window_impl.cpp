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
#include "stage_map_load.hpp"
#include "stage_map_save.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/optional.hpp>
#include <algorithm>
#include <utility>

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

const hamigaki::uuid player_id(guids[0]);
const hamigaki::uuid up_lift_id(guids[1]);
const hamigaki::uuid down_lift_id(guids[2]);
const hamigaki::uuid enemy_o_id(guids[3]);
const hamigaki::uuid enemy_a_id(guids[4]);
const hamigaki::uuid enemy_p_id(guids[5]);
const hamigaki::uuid enemy_w_id(guids[6]);
const hamigaki::uuid brick_id(guids[7]);
const hamigaki::uuid coin_brick_id(guids[8]);
const hamigaki::uuid item_brick_id(guids[9]);
const hamigaki::uuid used_block_id(guids[10]);
const hamigaki::uuid coin_box_id(guids[11]);
const hamigaki::uuid item_box_id(guids[12]);
const hamigaki::uuid secret_coin_id(guids[13]);
const hamigaki::uuid left_down_id(guids[14]);
const hamigaki::uuid right_down_id(guids[15]);

} // namespace

class map_edit_window::impl
{
private:
    typedef std::pair<int,int> texture_pos;

public:
    explicit impl(::HWND handle)
        : handle_(handle), modified_(false), mouse_captured_(false)
    {
    }

    ~impl()
    {
    }

    void new_stage(int width, int height)
    {
        map_.width = width*32;
        map_.height = height*32;
        map_.elements.clear();
        modified_ = true;

        int vert_max = update_scroll_box().second;

        ::SetScrollPos(handle_, SB_HORZ, 0, TRUE);
        ::SetScrollPos(handle_, SB_VERT, vert_max, TRUE);

        ::InvalidateRect(handle_, 0, FALSE);
    }

    void load_stage(const std::string& filename)
    {
        if (boost::algorithm::iends_with(filename, ".agm-yh", std::locale("")))
            load_map_from_binary(filename.c_str(), map_);
        else
            load_map_from_text(filename.c_str(), map_);
        modified_ = false;

        int vert_max = update_scroll_box().second;

        ::SetScrollPos(handle_, SB_HORZ, 0, TRUE);
        ::SetScrollPos(handle_, SB_VERT, vert_max, TRUE);

        ::InvalidateRect(handle_, 0, FALSE);
    }

    void save_stage(const std::string& filename)
    {
        if (boost::algorithm::iends_with(filename, ".agm-yh", std::locale("")))
            save_map_to_binary(filename.c_str(), map_);
        else
            save_map_to_text(filename.c_str(), map_);
        modified_ = false;
    }

    void render()
    {
        if (!device_)
            connect_d3d_device();

        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        int min_x = horz_scroll_value()*32 + 16;
        int min_y = vert_scroll_value()*32;

        int max_x = min_x + cr.right;
        int max_y = min_y + cr.bottom;

        typedef map_elements::const_iterator iter_type;
        const map_elements& x_y = map_.elements;
        iter_type beg = x_y.lower_bound(std::make_pair(min_x, 0));
        iter_type end = x_y.upper_bound(std::make_pair(max_x, map_.height));

        int cursor_x = min_x + cursor_pos_.first*32;
        int cursor_y = min_y + cursor_pos_.second*32;

        device_.clear_target(D3DCOLOR_XRGB(0x77,0x66,0xDD));
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

                boost::optional<texture_pos> pos = get_texture_pos(type);
                if (pos)
                    draw_character((x-min_x)/32, (y-min_y)/32, *pos);
            }

            if (max_x > map_.width)
            {
                ::draw_rectangle(
                    device_,
                    map_.width-min_x+16.0f, 0.0f, 0.0f,
                    max_x-static_cast<float>(map_.width),
                    static_cast<float>(cr.bottom),
                    0xFFC0C0C0ul
                );
            }

            if (max_y > map_.height)
            {
                ::draw_rectangle(
                    device_,
                    0.0f, 0.0f, 0.0f,
                    static_cast<float>(cr.right),
                    max_y-static_cast<float>(map_.height),
                    0xFFC0C0C0ul
                );
            }

            if ((cursor_x < map_.width) && (cursor_y < map_.height))
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
        horz.nMax = map_.width/32 - 1;
        horz.nPage = cr.right / 32;
        ::SetScrollInfo(handle_, SB_HORZ, &horz, TRUE);

        ::SCROLLINFO vert = {};
        vert.cbSize = sizeof(vert);
        vert.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        vert.nMin = 0;
        vert.nMax = map_.height/32 - 1;
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

    void select_char(const hamigaki::uuid& c)
    {
        selected_char_ = c;
    }

    void put_char()
    {
        int x = (horz_scroll_value() + cursor_pos_.first) * 32 + 16;
        int y = (vert_scroll_value() + cursor_pos_.second) * 32;

        if ((x >= map_.width) || (y >= map_.height))
            return;

        typedef map_elements::iterator iter_type;
        map_elements& x_y = map_.elements;

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

        modified_ = true;
        ::InvalidateRect(handle_, 0, FALSE);
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
    hamigaki::uuid selected_char_;
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

    boost::optional<texture_pos> get_texture_pos(const hamigaki::uuid& type)
    {
        if (type == player_id)
            return texture_pos(32*1, 32*0);
        else if (type == up_lift_id)
            return texture_pos(32*2, 32*0);
        else if (type == down_lift_id)
            return texture_pos(32*3, 32*0);
        else if (type == enemy_o_id)
            return texture_pos(32*0, 32*1);
        else if (type == enemy_a_id)
            return texture_pos(32*1, 32*1);
        else if (type == enemy_p_id)
            return texture_pos(32*2, 32*1);
        else if (type == enemy_w_id)
            return texture_pos(32*3, 32*1);
        else if (type == brick_id)
            return texture_pos(32*0, 32*2);
        else if (type == coin_brick_id)
            return texture_pos(32*1, 32*2);
        else if (type == item_brick_id)
            return texture_pos(32*2, 32*2);
        else if (type == used_block_id)
            return texture_pos(32*3, 32*2);
        else if (type == coin_box_id)
            return texture_pos(32*0, 32*3);
        else if (type == item_box_id)
            return texture_pos(32*1, 32*3);
        else if (type == secret_coin_id)
            return texture_pos(32*2, 32*3);
        else if (type == left_down_id)
            return texture_pos(32*0, 32*4);
        else if (type == right_down_id)
            return texture_pos(32*1, 32*4);
        else
            return boost::optional<texture_pos>();
    }

    void draw_box(int x, int y, unsigned long color)
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        float left = static_cast<float>(x * 32);
        float bottom = static_cast<float>(cr.bottom - y * 32);
        float top = bottom - 32.0f;

        ::draw_rectangle(device_, left, top, 0.0f, 32.0f, 32.0f, color);
    }

    void draw_character(int x, int y, const texture_pos& pos)
    {
        ::RECT cr;
        ::GetClientRect(handle_, &cr);

        float left = static_cast<float>(x * 32);
        float bottom = static_cast<float>(cr.bottom - y * 32);
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
