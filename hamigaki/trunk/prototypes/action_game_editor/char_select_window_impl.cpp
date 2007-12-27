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
#include "sprite_info_cache.hpp"
#include "texture_cache.hpp"
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>

class char_select_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle), chars_(0), textures_(device_), modified_(false)
    {
    }

    ~impl()
    {
    }

    void set_characters(std::set<game_character_class>* chars)
    {
        if (!device_)
            connect_d3d_device();

        chars_ = chars;
        textures_.clear();
        sprites_.clear();
        modified_ = false;

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

            if (chars_)
            {
                typedef std::set<game_character_class>::iterator iter_type;
                int index = 1;
                iter_type end = chars_->end();
                for (iter_type i = chars_->begin(); i != end; ++i)
                {
                    int x = index % 4;
                    int y = index / 4;

                    if (i->icon.empty())
                    {
                        sprite_info_set& infos = sprites_[i->sprite];
                        const sprite_pattern& pattern =
                            infos.groups[sprite_form::normal].patterns.at(0);

                        draw_sprite(
                            device_,
                            static_cast<float>(x*32),
                            static_cast<float>(y*32),
                            0.0f,
                            32.0f, 32.0f,
                            textures_[infos.texture],
                            infos.width * pattern.x,
                            infos.height * pattern.y,
                            infos.width,
                            infos.height, 0
                        );
                    }
                    else
                    {
                        const rectangle<int>& r = i->icon_rect;

                        draw_sprite(
                            device_,
                            static_cast<float>(x*32),
                            static_cast<float>(y*32),
                            0.0f,
                            32.0f, 32.0f,
                            textures_[i->icon],
                            r.x, r.y, r.lx, r.ly, 0
                        );
                    }
                    ++index;
                }
            }

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

    game_character_class* selected_char() const
    {
        std::size_t index =
            static_cast<std::size_t>(cursor_pos_.first + cursor_pos_.second*4);

        if ((index != 0) && (chars_ != 0) && (--index < chars_->size()))
        {
            typedef std::set<game_character_class>::iterator iter_type;
            iter_type pos = chars_->begin();
            std::advance(pos, index);
            return &*pos;
        }
        else
            return 0;
    }

    bool modified() const
    {
        return modified_;
    }

    void modified(bool value)
    {
        modified_ = value;
    }

    bool insert(const game_character_class& c)
    {
        BOOST_ASSERT(chars_ != 0);

        namespace bll = boost::lambda;

        if (std::find_if(
                chars_->begin(), chars_->end(),
                bll::bind(&game_character_class::name, bll::_1) == c.name
            ) != chars_->end())
        {
            return false;
        }

        chars_->insert(c).first->modified = true;
        modified_ = true;
        ::InvalidateRect(handle_, 0, FALSE);
        return true;
    }

private:
    ::HWND handle_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    direct3d_texture9 cursor_texture_;
    texture_cache textures_;
    std::pair<int,int> cursor_pos_;
    std::set<game_character_class>* chars_;
    sprite_info_cache sprites_;
    bool modified_;

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
};

char_select_window::char_select_window(::HWND handle) : pimpl_(new impl(handle))
{
}

char_select_window::~char_select_window()
{
}

void char_select_window::set_characters(std::set<game_character_class>* chars)
{
    pimpl_->set_characters(chars);
}

void char_select_window::render()
{
    pimpl_->render();
}

void char_select_window::cursor_pos(int x, int y)
{
    pimpl_->cursor_pos(x, y);
}

game_character_class* char_select_window::selected_char() const
{
    return pimpl_->selected_char();
}

bool char_select_window::modified() const
{
    return pimpl_->modified();
}

void char_select_window::modified(bool value)
{
    pimpl_->modified(value);
}

bool char_select_window::insert(const game_character_class& c)
{
    return pimpl_->insert(c);
}
