// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "blink_effect.hpp"
#include "draw.hpp"
#include "direct3d9.hpp"
#include "game_character.hpp"
#include "player_routine.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"
#include "sprite_info.hpp"
#include "stage_map.hpp"
#include "straight_routine.hpp"
#include "vanish_routine.hpp"
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <cmath>
#include <list>

#if defined(HAMIGAKI_USE_KNOCK_BACK)
    #include "knock_back_routine.hpp"
#endif

class main_window::impl
{
private:
    typedef std::list<game_character> chara_list;
    typedef chara_list::iterator chara_iterator;

public:
    explicit impl(::HWND handle)
        : handle_(handle)
        , sound_(handle_), input_(handle_)
        , active_(false), last_time_(::GetTickCount()), frames_(0)
        , scroll_x_(0.0f)
    {
        load_sprite_info_set_from_text("man.txt", player_sprite_info_);
        load_sprite_info_set_from_text("ball.txt", ball_sprite_info_);
        load_sprite_info_set_from_text("fragment.txt", fragment_sprite_info_);

        reset_characters();

        sound_.play_bgm("bgm.ogg");
    }

    ~impl()
    {
    }

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

        map_texture_ = create_png_texture(device_, "map_chips.png");

        man_texture_ =
            create_png_texture(device_, player_sprite_info_.texture());
        ball_texture_ =
            create_png_texture(device_, ball_sprite_info_.texture());
        fragment_texture_ =
            create_png_texture(device_, fragment_sprite_info_.texture());

        last_time_ = ::GetTickCount();
    }

    void process_input()
    {
        input_command cmd;
        if (active_)
            cmd  = input_();

        const unsigned long table[] = { 16, 17, 17 };
        unsigned long now = ::GetTickCount();
        unsigned long elapsed = now - last_time_;
        while (elapsed >= table[frames_ % 3])
        {
            elapsed -= table[frames_ % 3];
            if (++frames_ == 60)
                frames_ = 0;
            this->process_input_impl(cmd);
        }
        last_time_ = now - elapsed;
    }

    void render()
    {
        int x1 = static_cast<int>(scroll_x_) / 32;
        int x2 = static_cast<int>(std::ceil((scroll_x_+640.0f) / 32.0f));

        device_.clear_target(D3DCOLOR_XRGB(0x77,0x66,0xDD));
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);

            for (int y = 0; y < 15; ++y)
            {
                for (int x = x1; x < x2; ++x)
                {
                    char c = map_(x, y);
                    if (c == '=')
                        draw_block(x, y);
                }
            }

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            device_.set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            device_.set_texture_stage_state(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

            std::for_each(
                enemies_.begin(), enemies_.end(),
                boost::bind(&impl::draw_character, this, _1)
            );

            draw_character(player_);

            std::for_each(
                particles_.begin(), particles_.end(),
                boost::bind(&impl::draw_character, this, _1)
            );
        }
        device_.present();
    }

    void active(bool val)
    {
        active_ = val;
    }

private:
    ::HWND handle_;
    sound_engine sound_;
    input_engine input_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    direct3d_texture9 map_texture_;
    direct3d_texture9 man_texture_;
    direct3d_texture9 ball_texture_;
    direct3d_texture9 fragment_texture_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
    stage_map map_;
    float scroll_x_;
    sprite_info_set player_sprite_info_;
    sprite_info_set ball_sprite_info_;
    sprite_info_set fragment_sprite_info_;
    game_character player_;
    chara_list enemies_;
    chara_list particles_;

    void reset_characters()
    {
        load_map_from_text("map.txt", map_);

        player_.sprite_infos = &player_sprite_info_;
        player_.texture = &man_texture_;

        player_.routine = routine_type(player_routine(map_, sound_));
        player_.tmp_routine = routine_type();
        player_.effect = effect_type();

        int x, y;
        boost::tie(x, y) = map_.player_position();

        sprite_info info =
            player_.sprite_infos->get_group(player_.form.type)[0];

        player_.position.x = static_cast<float>(x * 32 + info.left);
        player_.position.y = static_cast<float>(y * 32);
        player_.position.lx = static_cast<float>(info.width);
        player_.position.ly = static_cast<float>(info.height);
        player_.speed.vx = 0.0f;
        player_.speed.vy = 0.0f;
        scroll_x_ = 0.0f;

        enemies_.clear();
        for (int y = 0; y < map_.height(); ++y)
        {
            for (int x = 0; x < map_.width(); ++x)
            {
                if (map_(x, y) == 'o')
                {
                    game_character enemy;

                    const sprite_info& info =
                        ball_sprite_info_.get_group(enemy.form.type)[0];

                    enemy.routine = routine_type(&straight_routine);
                    enemy.tmp_routine = routine_type();
                    enemy.sprite_infos = &ball_sprite_info_;
                    enemy.texture = &ball_texture_;
                    enemy.position.x = static_cast<float>(x * 32 + info.left);
                    enemy.position.y = static_cast<float>(y * 32);
                    enemy.position.lx = static_cast<float>(info.width);
                    enemy.position.ly = static_cast<float>(info.height);
                    enemy.speed.vx = 0.0f;
                    enemy.speed.vy = 0.0f;
                    enemy.form.options = sprite_options::back;

                    enemies_.push_back(enemy);
                }
            }
        }

        particles_.clear();
    }

    void process_map_collisions()
    {
        if (player_.speed.vy > 0.0f)
        {
            rect& r = player_.position;

            rect old_rect = r;
            old_rect.y -= player_.speed.vy;

            int old_y = top_block(old_rect);
            int new_y = top_block(r);
            int x1 = left_block(r);
            int x2 = right_block(r);
            int x = static_cast<int>(r.x+r.lx*0.5f)/32;

            for (int y = old_y+1; y <= new_y; ++y)
            {
                if (map_(x, y) == '=')
                {
                    map_.erase(x, y);
                    r.y = static_cast<float>(y * 32) - r.ly;
                    player_.speed.vy = -player_.speed.vy * 0.5f;

                    float dx[] = { -4.0f, -4.0f, 20.0f, 20.0f };
                    float dy[] = { 32.0f, 64.0f, 32.0f, 64.0f };
                    float vx[] = { -2.0f, -2.0f,  2.0f,  2.0f };
                    float vy[] = {  2.0f,  4.0f,  2.0f,  4.0f };

                    for (std::size_t i = 0; i < 4; ++i)
                    {
                        game_character fr;

                        fr.routine = routine_type(vanish_routine(30));
                        fr.tmp_routine = routine_type();
                        fr.sprite_infos = &fragment_sprite_info_;
                        fr.texture = &fragment_texture_;
                        fr.position.x = static_cast<float>(x*32) + dx[i];
                        fr.position.y = static_cast<float>(y*32) + dy[i];
                        fr.position.lx = 0.0f;
                        fr.position.ly = 0.0f;
                        fr.speed.vx = vx[i];
                        fr.speed.vy = vy[i];

                        particles_.push_back(fr);
                    }

                    sound_.play_se("break_block.ogg");

                    break;
                }
                else if (find_horizontal_blocks(map_, y, x1, x2))
                {
                    r.x = static_cast<float>(x * 32);
                    break;
                }
            }
        }
    }

    void process_collisions()
    {
        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); )
        {
            chara_iterator next = boost::next(i);

            if (i->form.type == sprite_form::nform)
            {
                enemies_.erase(i);
                i = next;
                continue;
            }

            rect r = i->position;
            r.ly *= 0.5f;
            r.y += r.ly;

            const rect& r2 = player_.position;
            float x1 = r2.x;
            float y1 = r2.y;
            float x2 = r2.x + r2.lx;
            float y2 = r2.y + r2.ly;

            if ( (includes_point(r, x1, y1) || includes_point(r, x2, y1)) &&
                !(includes_point(r, x1, y2) || includes_point(r, x2, y2)) )
            {
                i->change_form(static_four_char_code<'S','T','M','P'>::value);
                i->routine = routine_type(vanish_routine(5));
                i->speed.vx = 0.0f;
                i->speed.vy = 0.0f;
                sound_.play_se("stomp.ogg");
                player_.speed.vy = 8.0f;
                if (player_.form.type != player_routine::duck_form)
                    player_.change_form(player_routine::jump_form);
            }
            else if (player_.effect.empty() && player_.tmp_routine.empty() &&
                intersect_rects(player_.position, i->position))
            {
#if !defined(HAMIGAKI_USE_KNOCK_BACK)
                player_.effect = effect_type(&blink_effect);
#else
                float dx = -4.0f;
                if ((player_.form.options & sprite_options::back) != 0)
                    dx = -dx;

                player_.tmp_routine = routine_type(knock_back_routine(10, dx));
                player_.form.type = player_routine::knock_back_form;
#endif
                sound_.play_se("damage.ogg");
            }

            i = next;
        }

        for (chara_iterator i = particles_.begin(); i != particles_.end(); )
        {
            chara_iterator next = boost::next(i);

            if (i->form.type == sprite_form::nform)
            {
                particles_.erase(i);
                i = next;
                continue;
            }
            i = next;
        }
    }

    void process_input_impl(const input_command& cmd)
    {
        if (cmd.reset)
            reset_characters();

        std::for_each(
            enemies_.begin(), enemies_.end(),
            boost::bind(&game_character::move, _1, cmd, boost::cref(map_))
        );

        player_.move(cmd, map_);

        std::for_each(
            particles_.begin(), particles_.end(),
            boost::bind(&game_character::move, _1, cmd, boost::cref(map_))
        );

        process_map_collisions();
        process_collisions();

        float center = player_.position.x + player_.position.lx * 0.5f;
        float right_end = static_cast<float>(map_.width()*32);

        scroll_x_ = center - 320.0f;
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + 640.0f > right_end)
            scroll_x_ = right_end - 640.0f;
    }

    void draw_character(const game_character& chara)
    {
        const sprite_info_set& infos = *(chara.sprite_infos);

        const std::vector<sprite_info>& group =
            infos.get_group(chara.form.type);

        std::size_t pattern = (chara.step % (15 * group.size())) / 15;
        const sprite_info& info = group[pattern];

        ::draw_sprite(
            device_,
            chara.position.x - info.left - scroll_x_,
            480.0f - chara.position.y - infos.height(),
            0.0f,
            *(chara.texture),
            info.x, info.y,
            infos.width(), infos.height(),
            chara.form.options, chara.color
        );
    }

    void draw_block(int x, int y)
    {
        ::draw_sprite(
            device_,
            static_cast<float>(x*32)-scroll_x_,
            static_cast<float>(480-32 - y*32),
            0.0f,
            map_texture_,
            0, 0, 32, 32, false
        );
    }
};

main_window::main_window(::HWND handle) : pimpl_(new impl(handle))
{
}

void main_window::connect_d3d_device()
{
    pimpl_->connect_d3d_device();
}

void main_window::process_input()
{
    pimpl_->process_input();
}

void main_window::render()
{
    pimpl_->render();
}

void main_window::active(bool val)
{
    pimpl_->active(val);
}
