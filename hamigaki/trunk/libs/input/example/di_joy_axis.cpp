// di_joy_axis.cpp: an example for DirectInput joystick axis

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

di::device_info find_joystick(input::direct_input_manager& dinput)
{
    typedef di::device_info_iterator iter_type;
    typedef std::pair<iter_type,iter_type> pair_type;

    pair_type r(dinput.devices(di::device_type::joystick));
    if (r.first == r.second)
        throw std::runtime_error("Error: joystick not found");

    return *r.first;
}

int main(int argc, char* argv[])
{
    try
    {
        input::direct_input_manager dinput(::GetModuleHandle(0));

        const di::device_info& info = find_joystick(dinput);

        std::cout << "use " << info.instance_name << std::endl;

        input::direct_input_joystick joy =
            dinput.create_joystick_device(info.instance_guid);

        unsigned long level = di::nonexclusive_level|di::background_level;
        joy.set_cooperative_level(::GetDesktopWindow(), level);

        std::cout << "Press A button to stop..." << std::endl;
        di::joystick_state state;
        do
        {
            ::Sleep(500);
            joy.get_state(state);
            std::cout << state.position.x << std::endl;
        } while ((state.buttons[0] & 0x80) == 0);

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
