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
#include <boost/array.hpp>
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

typedef boost::array<unsigned char,256> keyboard_state;

struct keyboard_offset
{
    static const unsigned long escape       = 0x01;
    static const unsigned long _1           = 0x02;
    static const unsigned long _2           = 0x03;
    static const unsigned long _3           = 0x04;
    static const unsigned long _4           = 0x05;
    static const unsigned long _5           = 0x06;
    static const unsigned long _6           = 0x07;
    static const unsigned long _7           = 0x08;
    static const unsigned long _8           = 0x09;
    static const unsigned long _9           = 0x0A;
    static const unsigned long _0           = 0x0B;
    static const unsigned long minus        = 0x0C;
    static const unsigned long equals       = 0x0D;
    static const unsigned long back         = 0x0E;
    static const unsigned long tab          = 0x0F;
    static const unsigned long q            = 0x10;
    static const unsigned long w            = 0x11;
    static const unsigned long e            = 0x12;
    static const unsigned long r            = 0x13;
    static const unsigned long t            = 0x14;
    static const unsigned long y            = 0x15;
    static const unsigned long u            = 0x16;
    static const unsigned long i            = 0x17;
    static const unsigned long o            = 0x18;
    static const unsigned long p            = 0x19;
    static const unsigned long lbracket     = 0x1A;
    static const unsigned long rbracket     = 0x1B;
    static const unsigned long return_      = 0x1C;
    static const unsigned long lcontrol     = 0x1D;
    static const unsigned long a            = 0x1E;
    static const unsigned long s            = 0x1F;
    static const unsigned long d            = 0x20;
    static const unsigned long f            = 0x21;
    static const unsigned long g            = 0x22;
    static const unsigned long h            = 0x23;
    static const unsigned long j            = 0x24;
    static const unsigned long k            = 0x25;
    static const unsigned long l            = 0x26;
    static const unsigned long semicolon    = 0x27;
    static const unsigned long apostrophe   = 0x28;
    static const unsigned long grave        = 0x29;
    static const unsigned long lshift       = 0x2A;
    static const unsigned long backslash    = 0x2B;
    static const unsigned long z            = 0x2C;
    static const unsigned long x            = 0x2D;
    static const unsigned long c            = 0x2E;
    static const unsigned long v            = 0x2F;
    static const unsigned long b            = 0x30;
    static const unsigned long n            = 0x31;
    static const unsigned long m            = 0x32;
    static const unsigned long comma        = 0x33;
    static const unsigned long period       = 0x34;
    static const unsigned long slash        = 0x35;
    static const unsigned long rshift       = 0x36;
    static const unsigned long multiply     = 0x37;
    static const unsigned long lmenu        = 0x38;
    static const unsigned long space        = 0x39;
    static const unsigned long capital      = 0x3A;
    static const unsigned long f1           = 0x3B;
    static const unsigned long f2           = 0x3C;
    static const unsigned long f3           = 0x3D;
    static const unsigned long f4           = 0x3E;
    static const unsigned long f5           = 0x3F;
    static const unsigned long f6           = 0x40;
    static const unsigned long f7           = 0x41;
    static const unsigned long f8           = 0x42;
    static const unsigned long f9           = 0x43;
    static const unsigned long f10          = 0x44;
    static const unsigned long numlock      = 0x45;
    static const unsigned long scroll       = 0x46;
    static const unsigned long numpad7      = 0x47;
    static const unsigned long numpad8      = 0x48;
    static const unsigned long numpad9      = 0x49;
    static const unsigned long subtract     = 0x4A;
    static const unsigned long numpad4      = 0x4B;
    static const unsigned long numpad5      = 0x4C;
    static const unsigned long numpad6      = 0x4D;
    static const unsigned long add          = 0x4E;
    static const unsigned long numpad1      = 0x4F;
    static const unsigned long numpad2      = 0x50;
    static const unsigned long numpad3      = 0x51;
    static const unsigned long numpad0      = 0x52;
    static const unsigned long decimal      = 0x53;
    static const unsigned long oem_102      = 0x56;
    static const unsigned long f11          = 0x57;
    static const unsigned long f12          = 0x58;
    static const unsigned long f13          = 0x64;
    static const unsigned long f14          = 0x65;
    static const unsigned long f15          = 0x66;
    static const unsigned long kana         = 0x70;
    static const unsigned long abnt_c1      = 0x73;
    static const unsigned long convert      = 0x79;
    static const unsigned long noconvert    = 0x7B;
    static const unsigned long yen          = 0x7D;
    static const unsigned long abnt_c2      = 0x7E;
    static const unsigned long numpadequals = 0x8D;
    static const unsigned long prevtrack    = 0x90;
    static const unsigned long at           = 0x91;
    static const unsigned long colon        = 0x92;
    static const unsigned long underline    = 0x93;
    static const unsigned long kanji        = 0x94;
    static const unsigned long stop         = 0x95;
    static const unsigned long ax           = 0x96;
    static const unsigned long unlabeled    = 0x97;
    static const unsigned long nexttrack    = 0x99;
    static const unsigned long numpadenter  = 0x9C;
    static const unsigned long rcontrol     = 0x9D;
    static const unsigned long mute         = 0xA0;
    static const unsigned long calculator   = 0xA1;
    static const unsigned long playpause    = 0xA2;
    static const unsigned long mediastop    = 0xA4;
    static const unsigned long volumedown   = 0xAE;
    static const unsigned long volumeup     = 0xB0;
    static const unsigned long webhome      = 0xB2;
    static const unsigned long numpadcomma  = 0xB3;
    static const unsigned long divide       = 0xB5;
    static const unsigned long sysrq        = 0xB7;
    static const unsigned long rmenu        = 0xB8;
    static const unsigned long pause        = 0xC5;
    static const unsigned long home         = 0xC7;
    static const unsigned long up           = 0xC8;
    static const unsigned long prior        = 0xC9;
    static const unsigned long left         = 0xCB;
    static const unsigned long right        = 0xCD;
    static const unsigned long end          = 0xCF;
    static const unsigned long down         = 0xD0;
    static const unsigned long next         = 0xD1;
    static const unsigned long insert       = 0xD2;
    static const unsigned long delete_      = 0xD3;
    static const unsigned long lwin         = 0xDB;
    static const unsigned long rwin         = 0xDC;
    static const unsigned long apps         = 0xDD;
    static const unsigned long power        = 0xDE;
    static const unsigned long sleep        = 0xDF;
    static const unsigned long wake         = 0xE3;
    static const unsigned long websearch    = 0xE5;
    static const unsigned long webfavorites = 0xE6;
    static const unsigned long webrefresh   = 0xE7;
    static const unsigned long webstop      = 0xE8;
    static const unsigned long webforward   = 0xE9;
    static const unsigned long webback      = 0xEA;
    static const unsigned long mycomputer   = 0xEB;
    static const unsigned long mail         = 0xEC;
    static const unsigned long mediaselect  = 0xED;
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

    unsigned long dead_zone();
    void dead_zone(unsigned long val);

    unsigned long saturation();
    void saturation(unsigned long val);

private:
    boost::shared_ptr< ::IDirectInputDevice2A> pimpl_;
    unsigned long how_;
    unsigned long key_;
};

class device_impl;

} // namespace direct_input

struct HAMIGAKI_INPUT_DECL direct_input_error_traits
{
    typedef long value_type;

    static std::string message(long code);
};

typedef hamigaki::system::system_error<
    direct_input_error_traits
> direct_input_error;


class HAMIGAKI_INPUT_DECL direct_input_keyboard
{
public:
    typedef direct_input::object_info_iterator object_info_iterator;

    explicit direct_input_keyboard(::IDirectInputDevice2A* p);
    ~direct_input_keyboard();

    void set_cooperative_level(void* hwnd, unsigned long level);

    std::pair<object_info_iterator,object_info_iterator>
    objects(const direct_input::object_type& type);

    std::pair<object_info_iterator,object_info_iterator> objects();

    direct_input::device_object object(unsigned long offset);
    direct_input::device_object object(const direct_input::object_type& type);

    void acquire();
    void unacquire();
    void get_state(direct_input::keyboard_state& state);

private:
    boost::shared_ptr<direct_input::device_impl> pimpl_;
};

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
    boost::shared_ptr<direct_input::device_impl> pimpl_;
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

    direct_input_keyboard create_keyboard_device(const uuid& instance);
    direct_input_keyboard create_keyboard_device();

    direct_input_joystick create_joystick_device(const uuid& instance);

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
