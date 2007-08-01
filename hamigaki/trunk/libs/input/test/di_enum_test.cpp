// di_enum_test.cpp: test case for the enumeration of DirectInput devices

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/input for library home page.

#include <hamigaki/input/direct_input.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <iostream>
#include <windows.h>

namespace input = hamigaki::input;
namespace di = input::direct_input;
namespace ut = boost::unit_test;

struct print_instance_name
{
    void operator()(const di::device_info& info) const
    {
        std::cout << info.instance_name << '\n';
    }
};

void enumerate_test()
{
    input::direct_input_manager dinput(::GetModuleHandle(0));

    typedef di::device_info_iterator iter_type;
    typedef std::pair<iter_type,iter_type> pair_type;

    pair_type r1(dinput.device_info_range(di::device_type::mouse));
    std::cout << "The installed DirectInput mice:\n";
    std::for_each(r1.first, r1.second, print_instance_name());
    std::cout << std::endl;

    pair_type r2(dinput.device_info_range(di::device_type::keyboard));
    std::cout << "The installed DirectInput keyboards:\n";
    std::for_each(r2.first, r2.second, print_instance_name());
    std::cout << std::endl;

    pair_type r3(dinput.device_info_range(di::device_type::joystick));
    std::cout << "The installed DirectInput joysticks:\n";
    std::for_each(r3.first, r3.second, print_instance_name());
    std::cout << std::endl;

    pair_type r4(dinput.device_info_range(di::device_type::other));
    std::cout << "The installed DirectInput other devices:\n";
    std::for_each(r4.first, r4.second, print_instance_name());
    std::cout << std::endl;
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("DirectInput enumeration test");
    test->add(BOOST_TEST_CASE(&enumerate_test));
    return test;
}
