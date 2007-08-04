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

HAMIGAKI_INPUT_DECL extern const unsigned long ff_actuator;
HAMIGAKI_INPUT_DECL extern const unsigned long ff_effect_trigger;
HAMIGAKI_INPUT_DECL extern const unsigned long polled;
HAMIGAKI_INPUT_DECL extern const unsigned long aspect_position;
HAMIGAKI_INPUT_DECL extern const unsigned long aspect_velocity;
HAMIGAKI_INPUT_DECL extern const unsigned long aspect_accel;
HAMIGAKI_INPUT_DECL extern const unsigned long aspect_force;

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


class object_type
{
public:
    static const boost::uint16_t all            = 0x0000;

    static const boost::uint16_t rel_axis       = 0x0001;
    static const boost::uint16_t abs_axis       = 0x0002;
    static const boost::uint16_t axis           = 0x0003;

    static const boost::uint16_t push_button    = 0x0004;
    static const boost::uint16_t toggle_button  = 0x0008;
    static const boost::uint16_t button         = 0x000C;

    static const boost::uint16_t pov            = 0x0010;

    static const boost::uint16_t collection     = 0x0040;
    static const boost::uint16_t no_data        = 0x0080;

    static const boost::uint16_t actuator       = 0x0100;
    static const boost::uint16_t effect_triger  = 0x0200;

    explicit object_type(boost::uint16_t t=all, boost::uint16_t n=0xFFFF)
        : type_(t), sequence_(n)
    {
    }

    boost::uint16_t type() const
    {
        return type_;
    }

    void type(boost::uint16_t t)
    {
        type_ = t;
    }

    boost::uint16_t sequence() const
    {
        return sequence_;
    }

    void sequence(boost::uint16_t n)
    {
        sequence_ = n;
    }

    unsigned long to_dword() const
    {
        unsigned long tmp = static_cast<unsigned long>(sequence_) << 8;

        tmp |= static_cast<unsigned long>(type_ & 0xFF00) << 16;
        tmp |= static_cast<unsigned long>(type_ & 0xFF);

        return tmp;
    }

    static object_type from_dword(unsigned long flags)
    {
        boost::uint16_t t =
            static_cast<boost::uint16_t>(
                ((flags >> 16) & 0xFF00) | (flags & 0xFF)
            );

        boost::uint16_t n =
            static_cast<boost::uint16_t>((flags >> 8) & 0xFFFF);

        return object_type(t, n);
    }

private:
    boost::uint16_t type_;
    boost::uint16_t sequence_;
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

struct joystick_offset
{
    static const unsigned long x = 0ul;
    static const unsigned long y = 4ul;
    static const unsigned long z = 8ul;
    static const unsigned long rx = 12ul;
    static const unsigned long ry = 16ul;
    static const unsigned long rz = 20ul;
    static const unsigned long slider0 = 24ul;
    static const unsigned long slider1 = 28ul;

    static const unsigned long pov0 = 32ul;
    static const unsigned long pov1 = 36ul;
    static const unsigned long pov2 = 40ul;
    static const unsigned long pov3 = 44ul;

    static const unsigned long vx = 176ul;
    static const unsigned long vy = 180ul;
    static const unsigned long vz = 184ul;
    static const unsigned long vrx = 188ul;
    static const unsigned long vry = 192ul;
    static const unsigned long vrz = 196ul;
    static const unsigned long vslider0 = 200ul;
    static const unsigned long vslider1 = 204ul;

    static const unsigned long ax = 208ul;
    static const unsigned long ay = 212ul;
    static const unsigned long az = 216ul;
    static const unsigned long arx = 220ul;
    static const unsigned long ary = 224ul;
    static const unsigned long arz = 228ul;
    static const unsigned long aslider0 = 232ul;
    static const unsigned long aslider1 = 236ul;

    static const unsigned long fx = 240ul;
    static const unsigned long fy = 244ul;
    static const unsigned long fz = 248ul;
    static const unsigned long frx = 252ul;
    static const unsigned long fry = 256ul;
    static const unsigned long frz = 260ul;
    static const unsigned long fslider0 = 264ul;
    static const unsigned long fslider1 = 268ul;

    static unsigned long button(unsigned long n)
    {
        return 44u + n;
    }
};

struct device_info
{
    uuid instance_guid;
    uuid product_guid;
    unsigned long type;
    std::string instance_name;
    std::string product_name;
    uuid ff_driver_guid;
    unsigned short usage_page;
    unsigned short usage;
};

typedef hamigaki::coroutines::generator<device_info> device_info_iterator;

struct object_info
{
    uuid type_guid;
    unsigned long offset;
    object_type type;
    unsigned long flags;
    std::string name;
    unsigned long max_force;
    unsigned long force_resolution;
    unsigned short collection_number;
    unsigned short designator_index;
    unsigned short usage_page;
    unsigned short usage;
    unsigned long dimension;
    unsigned short exponent;
};

typedef hamigaki::coroutines::generator<object_info> object_info_iterator;

struct HAMIGAKI_INPUT_DECL device_object
{
public:
    device_object(
        const boost::shared_ptr< ::IDirectInputDevice2A>& p,
        unsigned long how, unsigned long key);

    ~device_object();

    object_info info() const;

    std::pair<long,long> range();
    void range(long min_val, long max_val);

    unsigned long deadzone();
    void deadzone(unsigned long val);

    unsigned long saturation();
    void saturation(unsigned long val);

private:
    boost::shared_ptr< ::IDirectInputDevice2A> pimpl_;
    unsigned long how_;
    unsigned long key_;
};

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
    typedef direct_input::object_info_iterator object_info_iterator;

    explicit direct_input_joystick(::IDirectInputDevice2A* p);
    ~direct_input_joystick();

    void set_cooperative_level(void* hwnd, unsigned long level);

    std::pair<object_info_iterator,object_info_iterator>
    objects(const direct_input::object_type& type);

    std::pair<object_info_iterator,object_info_iterator> objects();

    direct_input::device_object object(unsigned long offset);
    direct_input::device_object object(const direct_input::object_type& type);

    bool auto_center();
    void auto_center(bool val);

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
    devices(
        direct_input::device_type::values type, unsigned long flags);

    std::pair<device_info_iterator,device_info_iterator>
    devices(direct_input::device_type::values type);

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
