// di_joy_enum_test.cpp: test case for the enumeration of DirectInput joysticks

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

struct print_name
{
    void operator()(const di::object_info& info) const
    {
        std::cout << info.name << '\n';
    }
};

class enum_objects
{
public:
    explicit enum_objects(input::direct_input_manager& dinput)
        : dinput_(dinput)
    {
    }

    void operator()(const di::device_info& info) const
    {
        std::cout << "[" << info.instance_name << "]\n";

        input::direct_input_joystick joy =
            dinput_.create_joystick_device(info.instance_guid);

        typedef di::object_info_iterator iter_type;
        typedef std::pair<iter_type,iter_type> pair_type;

        pair_type r(joy.objects());
        std::for_each(r.first, r.second, print_name());
        std::cout << std::endl;
    }

private:
    input::direct_input_manager& dinput_;
};

void enumerate_test()
{
    input::direct_input_manager dinput(::GetModuleHandle(0));

    typedef di::device_info_iterator iter_type;
    typedef std::pair<iter_type,iter_type> pair_type;

    pair_type r(dinput.devices(di::device_type::joystick));
    std::for_each(r.first, r.second, enum_objects(dinput));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test =
        BOOST_TEST_SUITE("DirectInput joystick enumeration test");
    test->add(BOOST_TEST_CASE(&enumerate_test));
    return test;
}
