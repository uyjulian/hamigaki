// input_engine.cpp: input engine for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "input_engine.hpp"
#include "joystick_config.hpp"
#include "key_codes.hpp"
#include "keyboard_config.hpp"
#include "keyboard.hpp"
#include <boost/optional.hpp>
#include <cmath>

#if defined(__linux__)
    #include "linux_joystick.hpp"
    #include <hamigaki/dec_format.hpp>
    #include <map>
    #include <unistd.h>
#endif

namespace
{

#if defined(__linux__)
boost::optional<hamigaki::linux_joystick>
open_joystick(const char* cfg_path, joystick_config& cfg)
{
    joystick_config_list cfg_list;
    load_joystick_config_list(cfg_path, cfg_list);

    typedef std::map<std::string,std::string> table_type;
    typedef table_type::iterator iter_type;
    table_type table;

    int first_index = -1;
    std::string first_device;
    for (int i = 0; i < 16; ++i)
    {
        std::string path = "/dev/input/js" + hamigaki::to_dec<char>(i);
        if (::access(path.c_str(), R_OK) != 0)
            continue;

        hamigaki::linux_joystick js(path.c_str());

        const std::string& name = js.name();
        std::string& old = table[name];
        if (old.empty())
            old = path;

        if (first_index == -1)
        {
            first_index = i;
            first_device = name;
        }
    }

    if (first_index == -1)
        return boost::optional<hamigaki::linux_joystick>();

    typedef joystick_config_list::const_iterator cfg_iter_type;
    for (cfg_iter_type it = cfg_list.begin(); it != cfg_list.end(); ++it)
    {
        const std::string& name = it->first;

        iter_type pos = table.find(name);
        if (pos != table.end())
        {
            cfg = it->second;
            return hamigaki::linux_joystick(pos->second.c_str());
        }
    }

    std::string path = "/dev/input/js" + hamigaki::to_dec<char>(first_index);
    append_joystick_config(cfg_path, first_device, cfg);
    return hamigaki::linux_joystick(path.c_str());
}
#endif // defined(__linux__)

} // namespace

class input_engine::impl
{
public:
    explicit impl(void* widget)
        : key_cfg_(load_keyboard_config("keyboard-config.txt"))
        , keyboard_(static_cast<GtkWidget*>(widget))
        , old_key_dx_(0.0f), old_key_dy_(0.0f)
        , old_key_lr_(false), old_key_ud_(false)
    {
#if defined(__linux__)
        joystick_ = open_joystick("joystick-config.txt", joy_cfg_);
#endif
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
    keyboard_config key_cfg_;
    hamigaki::keyboard keyboard_;
#if defined(__linux__)
    joystick_config joy_cfg_;
    boost::optional<hamigaki::linux_joystick> joystick_;
#endif
    float old_key_dx_;
    float old_key_dy_;
    bool old_key_lr_;
    bool old_key_ud_;

    input_command get_keyboard_command()
    {
        float dx = 0.0f;
        float dy = 0.0f;

        bool left  = keyboard_.pushed(hamigaki::key_left);
        bool right = keyboard_.pushed(hamigaki::key_right);
        bool up    = keyboard_.pushed(hamigaki::key_up);
        bool down  = keyboard_.pushed(hamigaki::key_down);

        left  = left  || keyboard_.pushed(hamigaki::key_numpad4);
        right = right || keyboard_.pushed(hamigaki::key_numpad6);
        up    = up    || keyboard_.pushed(hamigaki::key_numpad8);
        down  = down  || keyboard_.pushed(hamigaki::key_numpad2);

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
            cmd.jump = keyboard_.pushed(key_cfg_.jump);
        if (key_cfg_.dash != -1)
            cmd.dash = keyboard_.pushed(key_cfg_.dash);
        if (key_cfg_.punch != -1)
            cmd.punch = keyboard_.pushed(key_cfg_.punch);
        if (key_cfg_.reset != -1)
            cmd.reset = keyboard_.pushed(key_cfg_.reset);

        return cmd;
    }

#if defined(__linux__)
    input_command get_joystick_command()
    {
        input_command cmd;

        if (hamigaki::linux_joystick* ptr = joystick_.get_ptr())
        {
            ptr->update();

            // TODO : support configuration

            float dx = static_cast<float>(ptr->axis_value(0)) / 32767.0f;
            float dy = static_cast<float>(ptr->axis_value(1)) / 32767.0f;

            if (std::abs(dx) < 0.2f)
                dx = 0.0f;
            if (std::abs(dy) < 0.2f)
                dy = 0.0f;

            cmd.x = dx;
            cmd.y = dy;

            if (joy_cfg_.jump != -1)
                cmd.jump = ptr->button_value(joy_cfg_.jump);
            if (joy_cfg_.dash != -1)
                cmd.dash = ptr->button_value(joy_cfg_.dash);
            if (joy_cfg_.punch != -1)
                cmd.punch = ptr->button_value(joy_cfg_.punch);
            if (joy_cfg_.reset != -1)
                cmd.reset = ptr->button_value(joy_cfg_.reset);
        }

        return cmd;
    }
#else
    input_command get_joystick_command()
    {
        return input_command();
    }
#endif
};

input_engine::input_engine(void* widget) : pimpl_(new impl(widget))
{
}

input_engine::~input_engine()
{
}

input_command input_engine::operator()()
{
    return (*pimpl_)();
}
