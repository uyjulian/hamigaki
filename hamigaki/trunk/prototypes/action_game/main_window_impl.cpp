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
#include <hamigaki/input/direct_input.hpp>
#include <cmath>
#include <stdexcept>

namespace input = hamigaki::input;
namespace di = input::direct_input;
namespace coro = hamigaki::coroutines;

namespace
{

const long axis_range = 1000;

di::device_info find_joystick(input::direct_input_manager& dinput)
{
    typedef di::device_info_iterator iter_type;
    typedef std::pair<iter_type,iter_type> pair_type;

    pair_type r(dinput.devices(di::device_type::joystick));
    if (r.first == r.second)
        throw std::runtime_error("Error: joystick not found");

    return *r.first;
}

input::direct_input_joystick create_joystick(::HWND handle)
{
    input::direct_input_manager dinput(::GetModuleHandle(0));
    const di::device_info& info = find_joystick(dinput);
    return dinput.create_joystick_device(info.instance_guid);
}

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
};

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle)
        , sound_(handle_)
        , joystick_(create_joystick(handle_))
        , active_(false), last_time_(::GetTickCount()), frames_(0)
        , scroll_x_(0.0f)
    {
        sound_.play_bgm("bgm.ogg");

        unsigned long level = di::exclusive_level|di::foreground_level;
        joystick_.set_cooperative_level(handle_, level);

        di::device_object x_axis = joystick_.object(di::joystick_offset::x);
        x_axis.range(-axis_range, axis_range);
        x_axis.deadzone(2000);

        di::device_object y_axis = joystick_.object(di::joystick_offset::y);
        y_axis.range(-axis_range, axis_range);
        y_axis.deadzone(2000);

        load_map_from_text(map_, "map.txt");
        load_sprite_info_list_from_text(player_sprite_info_, "man.txt");
        load_sprite_info_list_from_text(ball_sprite_info_, "ball.txt");

        player_.sprite_infos = &player_sprite_info_;
        player_.texture = &man_texture_;

        player_.routine = routine_type(player_routine(map_, sound_));

        for (std::size_t i = 0; i < 2; ++i)
        {
            enemies_[i].routine = routine_type(&straight_routine);
            enemies_[i].sprite_infos = &ball_sprite_info_;
            enemies_[i].texture = &chara_texture_;
        }
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
        chara_texture_ = create_png_texture(device_, "chara.png");

        const sprite_info& info =
            player_.sprite_infos->get_group(player_.form)[0];
        int sprite_height = player_.sprite_infos->height();

        ::D3DSURFACE_DESC desc = man_texture_.description(0);
        player_.position.r.x = static_cast<float>(32 + info.left);
        player_.position.r.y = static_cast<float>(448-sprite_height+info.top);
        player_.position.r.lx = static_cast<float>(info.width);
        player_.position.r.ly = static_cast<float>(info.height);
        player_.position.vx = 0.0f;
        player_.position.vy = 0.0f;

        desc = chara_texture_.description(0);
        enemies_[0].position.r.x = 608.0f;
        enemies_[0].position.r.y = 288.0f - static_cast<float>(desc.Height);
        enemies_[0].position.r.lx = static_cast<float>(desc.Width);
        enemies_[0].position.r.ly = static_cast<float>(desc.Height);
        enemies_[0].position.vx = 0.0f;
        enemies_[0].position.vy = 0.0f;

        enemies_[1].position.r.x = 1024.0f;
        enemies_[1].position.r.y = 320.0f - static_cast<float>(desc.Height);
        enemies_[1].position.r.lx = static_cast<float>(desc.Width);
        enemies_[1].position.r.ly = static_cast<float>(desc.Height);
        enemies_[1].position.vx = 0.0f;
        enemies_[1].position.vy = 0.0f;
    }

    void process_input()
    {
        di::joystick_state state = {};
        update_input_state(state);

        const unsigned long table[] = { 16, 17, 17 };
        unsigned long now = ::GetTickCount();
        unsigned long elapsed = now - last_time_;
        while (elapsed >= table[frames_ % 3])
        {
            elapsed -= table[frames_ % 3];
            if (++frames_ == 60)
                frames_ = 0;
            this->process_input_impl(state);
        }
        last_time_ = now - elapsed;
    }

    void render()
    {
        int x1 = static_cast<int>(scroll_x_) / 32;
        int x2 = static_cast<int>(std::ceil((scroll_x_+640.0f) / 32.0f));

        device_.clear_target(D3DCOLOR_XRGB(0,0,255));
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

            for (std::size_t i = 0; i < 2; ++i)
                draw_character(enemies_[i]);

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
    input::direct_input_joystick joystick_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    direct3d_texture9 man_texture_;
    direct3d_texture9 chara_texture_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
    stage_map map_;
    float scroll_x_;
    sprite_info_list player_sprite_info_;
    sprite_info_list ball_sprite_info_;
    game_character player_;
    game_character enemies_[2];

    void update_input_state(di::joystick_state& state)
    {
        if (!active_)
            return;

        try
        {
            joystick_.get_state(state);
        }
        catch (const input::direct_input_error& e)
        {
            if (e.code() == E_ACCESSDENIED)
            {
                active_ = false;
                return;
            }
            throw;
        }
    }

    void process_input_impl(const di::joystick_state& state)
    {
        if ((state.buttons[6] & 0x80) != 0)
        {
            const sprite_info& info = player_.sprite_infos->get_group(0)[0];
            int sprite_height = player_.sprite_infos->height();
            player_.position.r.x = static_cast<float>(32 + info.left);
            player_.position.r.y =
                static_cast<float>(448-sprite_height+info.top);
            player_.position.r.lx = static_cast<float>(info.width);
            player_.position.r.ly = static_cast<float>(info.height);
            player_.position.vx = 0.0f;
            player_.position.vy = 0.0f;
            player_.form = 0;
            player_.step = 0;
            player_.back = false;
        }

        float r = static_cast<float>(axis_range);
        float dx = static_cast<float>(state.position.x)/r;
        float dy = static_cast<float>(state.position.y)/r;

        float radius = std::sqrt(dx*dx + dy*dy);
        if (radius > 1.0f)
        {
            dx /= radius;
            dy /= radius;
        }

        input_command cmd;
        cmd.x = dx;
        cmd.y = dy;
        cmd.jump = (state.buttons[0] & 0x80) != 0;
        cmd.dash = (state.buttons[2] & 0x80) != 0;
        cmd.reset = (state.buttons[6] & 0x80) != 0;

        acceleration a;
        std::size_t form;

        boost::tie(a, form) = player_.routine(player_.position, cmd);

        std::size_t old_form = player_.form;
        player_.form = form;

        if (old_form != player_.form)
        {
            const sprite_info& old =
                player_.sprite_infos->get_group(old_form)[0];
            const sprite_info& cur =
                player_.sprite_infos->get_group(player_.form)[0];
            player_.position.change_form(old, cur);
            player_.step = 0;
        }
        else
            ++(player_.step);

        player_.position.move(a, map_);

        for (std::size_t i = 0; i < 2; ++i)
        {
            move_info& pos = enemies_[i].position;
            pos.move(enemies_[i].routine(pos, cmd).first, map_);
        }

        if (player_.position.vx >= 1.0f)
            player_.back = false;
        else if (player_.position.vx <= -1.0f)
            player_.back = true;

        float center = player_.position.r.x + player_.position.r.lx * 0.5f;
        float right_end = static_cast<float>(map_.width()*32);

        scroll_x_ = center - 320.0f;
        if (scroll_x_ < 0.0f)
            scroll_x_ = 0.0f;
        else if (scroll_x_ + 640.0f > right_end)
            scroll_x_ = right_end - 640.0f;
    }

    void draw_sprite(float x, float y, direct3d_texture9& texture)
    {
        ::draw_sprite(device_, x-scroll_x_, y, 0.0f, texture);
    }

    void draw_sprite(
        float x, float y,
        direct3d_texture9& texture, int tx, int ty, int tw, int th, bool back)
    {
        ::draw_sprite(
            device_, x-scroll_x_, y, 0.0f, texture, tx, ty, tw, th, back);
    }

    void draw_character(const game_character& chara)
    {
        const sprite_info_list& infos = *(chara.sprite_infos);
        const std::vector<sprite_info>& group = infos.get_group(chara.form);

        std::size_t pattern = (chara.step % (15 * group.size())) / 15;
        const sprite_info& info = group[pattern];

        draw_sprite(
            chara.position.r.x - info.left,
            chara.position.r.y - info.top,
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
            static_cast<float>(x*32)-scroll_x_, static_cast<float>(y*32), 0.0f,
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
