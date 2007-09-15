// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "draw.hpp"
#include "direct3d9.hpp"
#include "player_routine.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"
#include "sprite_info.hpp"
#include "stage_map.hpp"
#include "straight_routine.hpp"
#include <boost/bind.hpp>
#include <boost/next_prior.hpp>
#include <cmath>
#include <list>
#include <stdexcept>

namespace coro = hamigaki::coroutines;

namespace
{

struct game_character
{
    move_info position;
    routine_type routine;
    std::size_t form;
    int step;
    bool back;
    sprite_info_list* sprite_infos;
    direct3d_texture9* texture;

    game_character() : form(0), step(0), back(false)
    {
    }

    void move(const input_command& cmd, const stage_map& map)
    {
        acceleration a;
        std::size_t f;

        boost::tie(a, f) = routine(position, cmd);

        std::size_t old_form = form;
        form = f;

        if (old_form != form)
        {
            const sprite_info& old = sprite_infos->get_group(old_form)[0];
            const sprite_info& cur = sprite_infos->get_group(form)[0];
            position.change_form(old, cur);
            step = 0;
        }
        else
            ++(step);

        position.move(a, map);

        if (position.vx >= 1.0f)
            back = false;
        else if (position.vx <= -1.0f)
            back = true;
    }
};

} // namespace

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
        load_map_from_text(map_, "map.txt");
        load_sprite_info_list_from_text(player_sprite_info_, "man.txt");
        load_sprite_info_list_from_text(ball_sprite_info_, "ball.txt");

        player_.sprite_infos = &player_sprite_info_;
        player_.texture = &man_texture_;

        player_.routine = routine_type(player_routine(map_, sound_));

        int x, y;
        boost::tie(x, y) = map_.player_position();

        sprite_info info = player_.sprite_infos->get_group(player_.form)[0];
        player_.position.r.x = static_cast<float>(x * 32 + info.left);
        player_.position.r.y = static_cast<float>(y * 32);
        player_.position.r.lx = static_cast<float>(info.width);
        player_.position.r.ly = static_cast<float>(info.height);
        player_.position.vx = 0.0f;
        player_.position.vy = 0.0f;

        for (int y = 0; y < map_.height(); ++y)
        {
            for (int x = 0; x < map_.width(); ++x)
            {
                if (map_(x, y) == 'o')
                {
                    const sprite_info& info = ball_sprite_info_.get_group(0)[0];

                    game_character enemy;
                    enemy.routine = routine_type(&straight_routine);
                    enemy.sprite_infos = &ball_sprite_info_;
                    enemy.texture = &ball_texture_;
                    enemy.position.r.x = static_cast<float>(x * 32 + info.left);
                    enemy.position.r.y = static_cast<float>(y * 32);
                    enemy.position.r.lx = static_cast<float>(info.width);
                    enemy.position.r.ly = static_cast<float>(info.height);
                    enemy.position.vx = 0.0f;
                    enemy.position.vy = 0.0f;
                    enemy.back = true;

                    enemies_.push_back(enemy);
                }
            }
        }

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

        man_texture_ = create_png_texture(device_, "man.png");
        ball_texture_ = create_png_texture(device_, "ball.png");

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

            std::for_each(
                enemies_.begin(), enemies_.end(),
                boost::bind(&impl::draw_character, this, _1)
            );

            draw_character(player_);
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
    direct3d_texture9 man_texture_;
    direct3d_texture9 ball_texture_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
    stage_map map_;
    float scroll_x_;
    sprite_info_list player_sprite_info_;
    sprite_info_list ball_sprite_info_;
    game_character player_;
    chara_list enemies_;

    void process_input_impl(const input_command& cmd)
    {
        if (cmd.reset)
        {
            int x, y;
            boost::tie(x, y) = map_.player_position();

            const sprite_info& info = player_.sprite_infos->get_group(0)[0];
            player_.position.r.x = static_cast<float>(x * 32 + info.left);
            player_.position.r.y = static_cast<float>(y * 32);
            player_.position.r.lx = static_cast<float>(info.width);
            player_.position.r.ly = static_cast<float>(info.height);
            player_.position.vx = 0.0f;
            player_.position.vy = 0.0f;
            player_.form = 0;
            player_.step = 0;
            player_.back = false;
        }

        std::for_each(
            enemies_.begin(), enemies_.end(),
            boost::bind(&game_character::move, _1, cmd, boost::cref(map_))
        );

        player_.move(cmd, map_);

        for (chara_iterator i = enemies_.begin(); i != enemies_.end(); )
        {
            chara_iterator next = boost::next(i);

            rect r = i->position.r;
            r.ly *= 0.5f;
            r.y += r.ly;

            const rect& r2 = player_.position.r;
            float x1 = r2.x;
            float y1 = r2.y;
            float x2 = r2.x + r2.lx;
            float y2 = r2.y + r2.ly;

            if ( (includes_point(r, x1, y1) || includes_point(r, x2, y1)) &&
                !(includes_point(r, x1, y2) || includes_point(r, x2, y2)) )
            {
                enemies_.erase(i);
                sound_.play_se("stomp.ogg");
            }

            i = next;
        }

        float center = player_.position.r.x + player_.position.r.lx * 0.5f;
        float right_end = static_cast<float>(map_.width()*32);

        scroll_x_ = center - 320.0f;
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + 640.0f > right_end)
            scroll_x_ = right_end - 640.0f;
    }

    void draw_character(const game_character& chara)
    {
        const sprite_info_list& infos = *(chara.sprite_infos);
        const std::vector<sprite_info>& group = infos.get_group(chara.form);

        std::size_t pattern = (chara.step % (15 * group.size())) / 15;
        const sprite_info& info = group[pattern];

        ::draw_sprite(
            device_,
            chara.position.r.x - info.left - scroll_x_,
            480.0f - chara.position.r.y - infos.height(),
            0.0f,
            *(chara.texture),
            info.x, info.y,
            infos.width(), infos.height(),
            chara.back
        );
    }

    void draw_block(int x, int y)
    {
        draw_rectangle(
            device_,
            static_cast<float>(x*32)-scroll_x_,
            static_cast<float>(480-32 - y*32),
            0.0f,
            32.0f, 32.0f, D3DCOLOR_XRGB(0xAA,0x55,0x33));
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
