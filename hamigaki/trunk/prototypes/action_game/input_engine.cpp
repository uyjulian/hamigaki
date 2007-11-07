// input_engine.cpp: input engine for action_game

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "input_engine.hpp"
#include "joystick_config.hpp"
#include "keyboard_config.hpp"
#include <hamigaki/input/direct_input.hpp>
#include <boost/optional.hpp>
#include <cmath>
#include <windows.h>

namespace input = hamigaki::input;
namespace di = input::direct_input;

namespace
{

const long axis_range = 1000;
const unsigned long dead_zone = 2000;

inline bool button_is_down(unsigned char data)
{
    return (data & 0x80) != 0;
}

bool find_joystick(
    input::direct_input_manager& dinput, const hamigaki::uuid& guid)
{
    di::device_info_iterator beg, end;
    boost::tie(beg,end) = dinput.devices(di::device_type::joystick);

    for ( ; beg != end; ++beg)
    {
        if (beg->instance_guid == guid)
            return true;
    }

    return false;
}

boost::optional<input::direct_input_joystick>
open_joystick(
    input::direct_input_manager& dinput,
    const char* cfg_path, joystick_config& cfg)
{
    joystick_config_list cfg_list;
    load_joystick_config_list(cfg_path, cfg_list);

    typedef joystick_config_list::const_iterator cfg_iter_type;
    for (cfg_iter_type it = cfg_list.begin(); it != cfg_list.end(); ++it)
    {
        const hamigaki::uuid& guid = it->first;

        if (find_joystick(dinput, guid))
        {
            cfg = it->second;
            return dinput.create_joystick_device(guid);
        }
    }

    di::device_info_iterator beg, end;
    boost::tie(beg,end) = dinput.devices(di::device_type::joystick);
    if (beg != end)
    {
        append_joystick_config(cfg_path, beg->instance_guid, cfg);
        return dinput.create_joystick_device(beg->instance_guid);
    }

    return boost::optional<input::direct_input_joystick>();
}

} // namespace

class input_engine::impl
{
public:
    explicit impl(void* handle)
        : dinput_(::GetModuleHandle(0))
        , key_cfg_(load_keyboard_config("keyboard-config.txt"))
        , keyboard_(dinput_.create_keyboard_device())
        , old_key_dx_(0.0f), old_key_dy_(0.0f)
        , old_key_lr_(false), old_key_ud_(false)
    {
        unsigned long level = di::nonexclusive_level|di::foreground_level;
        keyboard_.set_cooperative_level(handle, level);

        joystick_ = open_joystick(dinput_, "joystick-config.txt", joy_cfg_);

        if (input::direct_input_joystick* joy = joystick_.get_ptr())
        {
            unsigned long level = di::exclusive_level|di::foreground_level;
            joy->set_cooperative_level(handle, level);

            di::device_object x_axis = joy->object(di::joystick_offset::x);
            x_axis.range(-axis_range, axis_range);
            x_axis.dead_zone(dead_zone);

            di::device_object y_axis = joy->object(di::joystick_offset::y);
            y_axis.range(-axis_range, axis_range);
            y_axis.dead_zone(dead_zone);
        }
    }

    input_command operator()()
    {
        input_command key_cmd = get_keyboard_command();
        input_command joy_cmd = get_joystick_command();

        input_command cmd = key_cmd;

        if ((key_cmd.x == 0.0f) && (joy_cmd.x != 0.0f))
            cmd.x = joy_cmd.x;
        if ((key_cmd.y == 0.0f) && (joy_cmd.y != 0.0f))
            cmd.y = joy_cmd.y;

        cmd.jump  = cmd.jump  || joy_cmd.jump;
        cmd.dash  = cmd.dash  || joy_cmd.dash;
        cmd.punch = cmd.punch || joy_cmd.punch;
        cmd.reset = cmd.reset || joy_cmd.reset;

        return cmd;
    }

private:
    input::direct_input_manager dinput_;
    keyboard_config key_cfg_;
    input::direct_input_keyboard keyboard_;
    joystick_config joy_cfg_;
    boost::optional<input::direct_input_joystick> joystick_;
    float old_key_dx_;
    float old_key_dy_;
    bool old_key_lr_;
    bool old_key_ud_;

    void update_keyboard_state(di::keyboard_state& state)
    {
        try
        {
            keyboard_.get_state(state);
        }
        catch (const input::direct_input_error& e)
        {
            if (e.code() == E_ACCESSDENIED)
                return;
            throw;
        }
    }

    void update_joystick_state(di::joystick_state& state)
    {
        try
        {
            if (input::direct_input_joystick* joy = joystick_.get_ptr())
                joy->get_state(state);
        }
        catch (const input::direct_input_error& e)
        {
            if (e.code() == E_ACCESSDENIED)
                return;
            throw;
        }
    }

    input_command get_keyboard_command()
    {
        di::keyboard_state state = {};
        this->update_keyboard_state(state);

        float dx = 0.0f;
        float dy = 0.0f;

        bool left  = button_is_down(state[di::keyboard_offset::left ]);
        bool right = button_is_down(state[di::keyboard_offset::right]);
        bool up    = button_is_down(state[di::keyboard_offset::up   ]);
        bool down  = button_is_down(state[di::keyboard_offset::down ]);

        left  = left  || button_is_down(state[di::keyboard_offset::numpad4]);
        right = right || button_is_down(state[di::keyboard_offset::numpad6]);
        up    = up    || button_is_down(state[di::keyboard_offset::numpad8]);
        down  = down  || button_is_down(state[di::keyboard_offset::numpad2]);

        if (left && right)
        {
            if (old_key_lr_)
                dx = old_key_dx_;
            else
                dx = -old_key_dx_;
            old_key_lr_ = true;
        }
        else
        {
            if (left)
                dx = -1.0f;
            if (right)
                dx = 1.0f;
            old_key_lr_ = false;
        }

        if (up && down)
        {
            if (old_key_ud_)
                dy = old_key_dy_;
            else
                dy = -old_key_dy_;
            old_key_ud_ = true;
        }
        else
        {
            if (down)
                dy = -1.0f;
            if (up)
                dy = 1.0f;
            old_key_ud_ = false;
        }

        old_key_dx_ = dx;
        old_key_dy_ = dy;

        float radius = std::sqrt(dx*dx + dy*dy);
        if (radius > 1.0f)
        {
            dx /= radius;
            dy /= radius;
        }

        input_command cmd;
        cmd.x = dx;
        cmd.y = dy;

        if (key_cfg_.jump != -1)
            cmd.jump = button_is_down(state[key_cfg_.jump]);
        if (key_cfg_.dash != -1)
            cmd.dash = button_is_down(state[key_cfg_.dash]);
        if (key_cfg_.punch != -1)
            cmd.punch = button_is_down(state[key_cfg_.punch]);
        if (key_cfg_.reset != -1)
            cmd.reset = button_is_down(state[key_cfg_.reset]);

        return cmd;
    }

    input_command get_joystick_command()
    {
        di::joystick_state state = {};
        this->update_joystick_state(state);

        float r = static_cast<float>(axis_range);
        float dx = static_cast<float>(state.position.x)/r;
        float dy = static_cast<float>(-state.position.y)/r;

        if (static_cast<unsigned>(state.pov_directions[0] & 0xFFFF) != 0xFFFF)
        {
            float pov = static_cast<float>(state.pov_directions[0]);
            float a = pov / 18000.0f * 3.1415927f;

            dx = std::sin(a);
            if (std::abs(dx) < 0.001)
                dx = 0.0f;

            dy = std::cos(a);
            if (std::abs(dy) < 0.001)
                dy = 0.0f;
        }

        float radius = std::sqrt(dx*dx + dy*dy);
        if (radius > 1.0f)
        {
            dx /= radius;
            dy /= radius;
        }

        input_command cmd;
        cmd.x = dx;
        cmd.y = dy;

        if (joy_cfg_.jump != -1)
            cmd.jump = button_is_down(state.buttons[joy_cfg_.jump]);
        if (joy_cfg_.dash != -1)
            cmd.dash = button_is_down(state.buttons[joy_cfg_.dash]);
        if (joy_cfg_.punch != -1)
            cmd.punch = button_is_down(state.buttons[joy_cfg_.punch]);
        if (joy_cfg_.reset != -1)
            cmd.reset = button_is_down(state.buttons[joy_cfg_.reset]);

        return cmd;
    }
};

input_engine::input_engine(void* handle) : pimpl_(new impl(handle))
{
}

input_engine::~input_engine()
{
}

input_command input_engine::operator()()
{
    return (*pimpl_)();
}
