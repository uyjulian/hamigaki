// direct_input.hpp: DirectInput devices

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/input for library home page.

#ifndef HAMIGAKI_INPUT_DIRECT_INPUT_HPP
#define HAMIGAKI_INPUT_DIRECT_INPUT_HPP

#include <hamigaki/input/detail/config.hpp>
#include <hamigaki/input/detail/auto_link.hpp>
#include <hamigaki/coroutine/generator.hpp>
#include <hamigaki/system/system_error.hpp>
#include <hamigaki/uuid.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <utility>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251 4275)
#endif

#if defined(BOOST_WINDOWS) && !defined(__GNUC__)
    #pragma comment(lib, "user32.lib")
#endif

struct IDirectInputDevice2A;

namespace hamigaki { namespace input {

namespace direct_input
{

HAMIGAKI_INPUT_DECL extern const unsigned long all_devices;
HAMIGAKI_INPUT_DECL extern const unsigned long attached_only;
HAMIGAKI_INPUT_DECL extern const unsigned long force_feedback_only;

HAMIGAKI_INPUT_DECL extern const unsigned long exclusive_level;
HAMIGAKI_INPUT_DECL extern const unsigned long nonexclusive_level;
HAMIGAKI_INPUT_DECL extern const unsigned long foreground_level;
HAMIGAKI_INPUT_DECL extern const unsigned long background_level;

struct device_type
{
    enum values
    {
        other = 1,
        mouse,
        keyboard,
        joystick
    };
};

struct vector3
{
    long x;
    long y;
    long z;
};

struct joystick_state
{
    vector3 position;
    vector3 rotation;
    long sliders[2];
    unsigned long pov_directions[4];
    unsigned char buttons[128];
    vector3 velocity;
    vector3 angular_velocity;
    long slider_velocities[2];
    vector3 acceleration;
    vector3 angular_acceleration;
    long slider_accelerations[2];
    vector3 force;
    vector3 torque;
    long slider_forces[2];
};

struct force_feedback_info
{
    uuid driver_guid;
    unsigned short usage_page;
    unsigned short usage;
};

struct device_info
{
    uuid instance_guid;
    uuid product_guid;
    unsigned long type;
    std::string instance_name;
    std::string product_name;
    force_feedback_info ff_info;
};

typedef hamigaki::coroutines::generator<device_info> device_info_iterator;

} // namespace direct_input

struct HAMIGAKI_INPUT_DECL direct_input_error_traits
{
    typedef long value_type;

    static std::string message(long code);
};

typedef hamigaki::system::system_error<
    direct_input_error_traits
> direct_input_error;


class HAMIGAKI_INPUT_DECL direct_input_joystick
{
public:
    explicit direct_input_joystick(::IDirectInputDevice2A* p);
    ~direct_input_joystick();

    void set_cooperative_level(void* hwnd, unsigned long level);

    void acquire();
    void unacquire();
    void get_state(direct_input::joystick_state& state);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

class HAMIGAKI_INPUT_DECL direct_input_manager
{
public:
    typedef direct_input::device_info_iterator device_info_iterator;

    explicit direct_input_manager(void* instance);
    ~direct_input_manager();

    std::pair<device_info_iterator,device_info_iterator>
    device_info_range(
        direct_input::device_type::values type, unsigned long flags);

    std::pair<device_info_iterator,device_info_iterator>
    device_info_range(direct_input::device_type::values type);

    direct_input_joystick create_joystick_device(const uuid& driver_guid);

private:
    class impl;
    boost::shared_ptr<impl> pimpl_;
};

} } // End namespaces input, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_INPUT_DIRECT_INPUT_HPP
