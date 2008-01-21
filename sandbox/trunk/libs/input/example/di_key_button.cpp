// di_key_button.cpp: an example for DirectInput keyboard button

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/input for library home page.

#include <hamigaki/input/direct_input.hpp>
#include <iostream>
#include <stdexcept>
#include <windows.h>

namespace input = hamigaki::input;
namespace di = input::direct_input;

int main(int argc, char* argv[])
{
    try
    {
        input::direct_input_manager dinput(::GetModuleHandle(0));

        input::direct_input_keyboard keyboard = dinput.create_keyboard_device();

        unsigned long level = di::nonexclusive_level|di::background_level;
        keyboard.set_cooperative_level(::GetDesktopWindow(), level);

        std::cout << "Press Escape key to stop..." << std::endl;
        di::keyboard_state state;
        do
        {
            ::Sleep(100);
            keyboard.get_state(state);
        } while ((state[di::keyboard_offset::escape] & 0x80) == 0);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
