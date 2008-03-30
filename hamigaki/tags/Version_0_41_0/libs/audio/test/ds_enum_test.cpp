// ds_enum_test.cpp: test case for DirectSoundEnumerate

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/direct_sound.hpp>
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <iostream>

namespace audio = hamigaki::audio;
namespace ds = audio::direct_sound;
namespace ut = boost::unit_test;

struct print_device_name
{
    void operator()(const ds::device_info& info) const
    {
        std::cout << info.description << '\n';
    }
};

void direct_sound_enumerate_test()
{
    std::pair<
        ds::device_info_iterator,
        ds::device_info_iterator> r(ds::device_info_range());

    std::cout << "The installed DirectSound drivers:\n";
    std::for_each(r.first, r.second, print_device_name());
    std::cout << std::endl;
}

void direct_sound_capture_enumerate_test()
{
    std::pair<
        ds::device_info_iterator,
        ds::device_info_iterator> r(ds::capture_device_info_range());

    std::cout << "The installed DirectSoundCapture drivers:\n";
    std::for_each(r.first, r.second, print_device_name());
    std::cout << std::endl;
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("DirectSoundEnumerate test");
    test->add(BOOST_TEST_CASE(&direct_sound_enumerate_test));
    test->add(BOOST_TEST_CASE(&direct_sound_capture_enumerate_test));
    return test;
}
