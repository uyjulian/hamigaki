// main_window.cpp: main window implementation for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "draw.hpp"
#include "direct3d9.hpp"
#include "png_loader.hpp"
#include "sprite.hpp"
#include <hamigaki/input/direct_input.hpp>
#include <cmath>
#include <stdexcept>

namespace input = hamigaki::input;
namespace di = input::direct_input;

namespace
{

const long axis_range = 1000;
const float max_speed = 2.0f;

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
        , joystick_(create_joystick(handle_))
        , active_(false), last_time_(::GetTickCount()), frames_(0)
        , x_(32.0f), y_(416.0f), vx_(0.0f), vy_(0.0f)
    {
        unsigned long level = di::exclusive_level|di::foreground_level;
        joystick_.set_cooperative_level(handle_, level);

        di::device_object x_axis = joystick_.object(di::joystick_offset::x);
        x_axis.range(-axis_range, axis_range);
        x_axis.deadzone(2000);

        di::device_object y_axis = joystick_.object(di::joystick_offset::y);
        y_axis.range(-axis_range, axis_range);
        y_axis.deadzone(2000);
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
    }

    void process_input()
    {
        if (!active_)
            return;

        di::joystick_state state;
        joystick_.get_state(state);

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

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
            device_.set_render_state(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            device_.set_render_state(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

            draw_sprite(device_, x_, y_, 0.0f, chara_texture_);

            device_.set_render_state(D3DRS_ALPHABLENDENABLE, FALSE);

            draw_rectangle(
                device_, 0.0f, 448.0f, 0.0f,
                640.0f, 32.0f, D3DCOLOR_XRGB(0xAA,0x55,0x33));
        }
        device_.present();
    }

    void active(bool val)
    {
        active_ = val;
    }

private:
    ::HWND handle_;
    input::direct_input_joystick joystick_;
    direct3d9 d3d_;
    direct3d_device9 device_;
    direct3d_texture9 chara_texture_;
    bool active_;
    unsigned long last_time_;
    unsigned frames_;
    float x_;
    float y_;
    float vx_;
    float vy_;

    void process_input_impl(const di::joystick_state& state)
    {
        bool push_jump = (state.buttons[0] & 0x80) != 0;

        float a = static_cast<float>(axis_range);
        float dx = static_cast<float>(state.position.x)/a;
        float dy = static_cast<float>(state.position.y)/a;

        float r = std::sqrt(dx*dx + dy*dy);
        if (r > 1.0f)
        {
            dx /= r;
            dy /= r;
        }

        dx *= max_speed;
        dy *= max_speed;

        float x_max = 608.0f;
        float y_max = 416.0f;

        x_ += dx;
        if (x_ < 0.0f)
            x_ = 0.0f;
        else if (x_ > x_max)
            x_ = x_max;

        if (y_ < y_max)
        {
            if (push_jump && (vy_ < 0.0))
                vy_ -= 0.2f;
            vy_ += 0.3f;

            if (vy_ > 5.0f)
                vy_ = 5.0f;
        }
        else if (push_jump)
            vy_ = -4.0f;

        y_ += vy_ * 2.0f;
        if (y_ < 0.0f)
            y_ = 0.0f;
        else if (y_ > y_max)
            y_ = y_max;
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
