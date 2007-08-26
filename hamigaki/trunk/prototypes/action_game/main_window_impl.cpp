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
#include "sound_engine.hpp"
#include "sprite.hpp"
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

} // namespace

class main_window::impl
{
public:
    explicit impl(::HWND handle)
        : handle_(handle)
        , sound_(handle_)
        , joystick_(create_joystick(handle_))
        , active_(false), last_time_(::GetTickCount()), frames_(0)
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

        player_routine_ = routine_type(&player_routine);
        enemy_routine_ = routine_type(&straight_routine);
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

        chara_texture_ =
            create_png_texture(device_, "chara.png");

        const ::D3DSURFACE_DESC& desc = chara_texture_.description(0);
        player_rect_.x = 32.0f;
        player_rect_.y = 448.0f - static_cast<float>(desc.Height);
        player_rect_.lx = static_cast<float>(desc.Width);
        player_rect_.ly = static_cast<float>(desc.Height);

        enemy_rect_.x = 608.0f;
        enemy_rect_.y = 288.0f;
        enemy_rect_.lx = static_cast<float>(desc.Width);
        enemy_rect_.ly = static_cast<float>(desc.Height);
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
        device_.clear_target(D3DCOLOR_XRGB(0,0,255));
        {
            scoped_scene scene(device_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);

            for (std::size_t y = 0; y < 15; ++y)
            {
                for (std::size_t x = 0; x < 20; ++x)
                {
                    char c = map_(x, y);
                    if (c == '=')
                        draw_block(x, y);
                }
            }

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            draw_sprite(device_,
                enemy_rect_.x, enemy_rect_.y, 0.0f, chara_texture_);

            draw_sprite(device_,
                player_rect_.x, player_rect_.y, 0.0f, chara_texture_);
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
    direct3d_texture9 chara_texture_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
    rect player_rect_;
    rect enemy_rect_;
    stage_map map_;
    routine_type player_routine_;
    routine_type enemy_routine_;

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
            const ::D3DSURFACE_DESC& desc = chara_texture_.description(0);
            player_rect_.x = 32.0f;
            player_rect_.y = 448.0f - static_cast<float>(desc.Height);
        }

        const ::D3DSURFACE_DESC& desc = chara_texture_.description(0);

        float a = static_cast<float>(axis_range);
        float dx = static_cast<float>(state.position.x)/a;
        float dy = static_cast<float>(state.position.y)/a;

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

        player_rect_ = player_routine_(player_rect_, cmd, &map_);
        enemy_rect_ = enemy_routine_(enemy_rect_, cmd, &map_);
    }

    void draw_block(std::size_t x, std::size_t y)
    {
        draw_rectangle(
            device_, static_cast<float>(x*32), static_cast<float>(y*32), 0.0f,
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
