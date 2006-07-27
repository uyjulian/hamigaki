//  ds_enum_test.cpp: test case for DirectSoundEnumerate

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <hamigaki/audio/direct_sound.hpp>
#include <boost/test/unit_test.hpp>
#include <iterator>
#include <vector>

namespace audio = hamigaki::audio;
namespace ds = audio::direct_sound;
namespace ut = boost::unit_test;

struct create_device
{
    bool operator()(const ds::device_info& info) const
    {
        audio::direct_sound_device dev(info.driver_guid);
        return true;
    }
};

struct always_true
{
    bool operator()(const ds::device_info&) const
    {
        return true;
    }
};

struct always_false
{
    bool operator()(const ds::device_info&) const
    {
        return false;
    }
};

void direct_sound_enumerate_test()
{
    audio::direct_sound_enumerate((create_device()));

    {
        std::vector<ds::device_info> vec;
        audio::direct_sound_enumerate_copy(std::back_inserter(vec));
    }

    BOOST_CHECK(audio::direct_sound_find_if((always_true())));
    BOOST_CHECK(!audio::direct_sound_find_if((always_false())));
}

ut::test_suite* init_unit_test_suite(int, char* [])
{
    ut::test_suite* test = BOOST_TEST_SUITE("DirectSoundEnumerate test");
    test->add(BOOST_TEST_CASE(&direct_sound_enumerate_test));
    return test;
}
