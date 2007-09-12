// di_joy_guid.cpp: print GUIDs of DirectInput joysticks

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

struct print_joystick_instance
{
    void operator()(const di::device_info& info) const
    {
        std::cout
            << info.instance_guid.to_guid_string() << ' '
            << info.instance_name << '\n';
    }
};

int main(int argc, char* argv[])
{
    try
    {
        input::direct_input_manager dinput(::GetModuleHandle(0));

        di::device_info_iterator beg, end;
        boost::tie(beg,end) = dinput.devices(di::device_type::joystick);

        std::for_each(beg, end, print_joystick_instance());
        std::cout << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}
