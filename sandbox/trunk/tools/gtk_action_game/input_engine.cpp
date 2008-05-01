// input_engine.cpp: input engine for action_game

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "input_engine.hpp"
#include "keyboard_config.hpp"
#include "keyboard.hpp"
#include <boost/optional.hpp>
#include <cmath>

class input_engine::impl
{
public:
    explicit impl(void* widget)
        : key_cfg_(load_keyboard_config("keyboard-config.txt"))
        , keyboard_(static_cast<GtkWidget*>(widget))
        , old_key_dx_(0.0f), old_key_dy_(0.0f)
        , old_key_lr_(false), old_key_ud_(false)
    {
    }

    input_command operator()()
    {
        return get_keyboard_command();
    }

private:
    keyboard_config key_cfg_;
    hamigaki::keyboard keyboard_;
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
