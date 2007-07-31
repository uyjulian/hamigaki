// direct_input_formats.cpp: DirectInput data formats

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/input for library home page.

#include <dinput.h>

#ifndef DIDFT_OPTIONAL
    #define DIDFT_OPTIONAL 0x80000000
#endif

namespace hamigaki { namespace input { namespace direct_input {

namespace
{

const ::GUID x_axis_guid =
{ 0xA36D02E0,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID y_axis_guid =
{ 0xA36D02E1,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID z_axis_guid =
{ 0xA36D02E2,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID rx_axis_guid =
{ 0xA36D02F4,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID ry_axis_guid =
{ 0xA36D02F5,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID rz_axis_guid =
{ 0xA36D02E3,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID slider_guid =
{ 0xA36D02E4,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID button_guid =
{ 0xA36D02F0,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID pov_guid =
{ 0xA36D02F2,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };


struct format
{
    static const ::DWORD axis   = DIDFT_AXIS  |DIDFT_ANYINSTANCE|DIDFT_OPTIONAL;
    static const ::DWORD pov    = DIDFT_POV   |DIDFT_ANYINSTANCE|DIDFT_OPTIONAL;
    static const ::DWORD button = DIDFT_BUTTON|DIDFT_ANYINSTANCE|DIDFT_OPTIONAL;
};

struct aspect
{
    static const ::DWORD position = DIDOI_ASPECTPOSITION;
    static const ::DWORD velocity = DIDOI_ASPECTVELOCITY;
    static const ::DWORD accel    = DIDOI_ASPECTACCEL;
    static const ::DWORD force    = DIDOI_ASPECTFORCE;
};

::DIOBJECTDATAFORMAT joystick_obj_formats[] =
{
    { &x_axis_guid,    0, format::axis,   aspect::position },
    { &y_axis_guid,    4, format::axis,   aspect::position },
    { &z_axis_guid,    8, format::axis,   aspect::position },
    { &rx_axis_guid,  12, format::axis,   aspect::position },
    { &ry_axis_guid,  16, format::axis,   aspect::position },
    { &rz_axis_guid,  20, format::axis,   aspect::position },
    { &slider_guid,   24, format::axis,   aspect::position },
    { &slider_guid,   28, format::axis,   aspect::position },
    { &pov_guid,      32, format::pov,    0                },
    { &pov_guid,      36, format::pov,    0                },
    { &pov_guid,      40, format::pov,    0                },
    { &pov_guid,      44, format::pov,    0                },
    { 0,              48, format::button, 0                },
    { 0,              49, format::button, 0                },
    { 0,              50, format::button, 0                },
    { 0,              51, format::button, 0                },
    { 0,              52, format::button, 0                },
    { 0,              53, format::button, 0                },
    { 0,              54, format::button, 0                },
    { 0,              55, format::button, 0                },
    { 0,              56, format::button, 0                },
    { 0,              57, format::button, 0                },
    { 0,              58, format::button, 0                },
    { 0,              59, format::button, 0                },
    { 0,              60, format::button, 0                },
    { 0,              61, format::button, 0                },
    { 0,              62, format::button, 0                },
    { 0,              63, format::button, 0                },
    { 0,              64, format::button, 0                },
    { 0,              65, format::button, 0                },
    { 0,              66, format::button, 0                },
    { 0,              67, format::button, 0                },
    { 0,              68, format::button, 0                },
    { 0,              69, format::button, 0                },
    { 0,              70, format::button, 0                },
    { 0,              71, format::button, 0                },
    { 0,              72, format::button, 0                },
    { 0,              73, format::button, 0                },
    { 0,              74, format::button, 0                },
    { 0,              75, format::button, 0                },
    { 0,              76, format::button, 0                },
    { 0,              77, format::button, 0                },
    { 0,              78, format::button, 0                },
    { 0,              79, format::button, 0                },
    { 0,              80, format::button, 0                },
    { 0,              81, format::button, 0                },
    { 0,              82, format::button, 0                },
    { 0,              83, format::button, 0                },
    { 0,              84, format::button, 0                },
    { 0,              85, format::button, 0                },
    { 0,              86, format::button, 0                },
    { 0,              87, format::button, 0                },
    { 0,              88, format::button, 0                },
    { 0,              89, format::button, 0                },
    { 0,              90, format::button, 0                },
    { 0,              91, format::button, 0                },
    { 0,              92, format::button, 0                },
    { 0,              93, format::button, 0                },
    { 0,              94, format::button, 0                },
    { 0,              95, format::button, 0                },
    { 0,              96, format::button, 0                },
    { 0,              97, format::button, 0                },
    { 0,              98, format::button, 0                },
    { 0,              99, format::button, 0                },
    { 0,             100, format::button, 0                },
    { 0,             101, format::button, 0                },
    { 0,             102, format::button, 0                },
    { 0,             103, format::button, 0                },
    { 0,             104, format::button, 0                },
    { 0,             105, format::button, 0                },
    { 0,             106, format::button, 0                },
    { 0,             107, format::button, 0                },
    { 0,             108, format::button, 0                },
    { 0,             109, format::button, 0                },
    { 0,             110, format::button, 0                },
    { 0,             111, format::button, 0                },
    { 0,             112, format::button, 0                },
    { 0,             113, format::button, 0                },
    { 0,             114, format::button, 0                },
    { 0,             115, format::button, 0                },
    { 0,             116, format::button, 0                },
    { 0,             117, format::button, 0                },
    { 0,             118, format::button, 0                },
    { 0,             119, format::button, 0                },
    { 0,             120, format::button, 0                },
    { 0,             121, format::button, 0                },
    { 0,             122, format::button, 0                },
    { 0,             123, format::button, 0                },
    { 0,             124, format::button, 0                },
    { 0,             125, format::button, 0                },
    { 0,             126, format::button, 0                },
    { 0,             127, format::button, 0                },
    { 0,             128, format::button, 0                },
    { 0,             129, format::button, 0                },
    { 0,             130, format::button, 0                },
    { 0,             131, format::button, 0                },
    { 0,             132, format::button, 0                },
    { 0,             133, format::button, 0                },
    { 0,             134, format::button, 0                },
    { 0,             135, format::button, 0                },
    { 0,             136, format::button, 0                },
    { 0,             137, format::button, 0                },
    { 0,             138, format::button, 0                },
    { 0,             139, format::button, 0                },
    { 0,             140, format::button, 0                },
    { 0,             141, format::button, 0                },
    { 0,             142, format::button, 0                },
    { 0,             143, format::button, 0                },
    { 0,             144, format::button, 0                },
    { 0,             145, format::button, 0                },
    { 0,             146, format::button, 0                },
    { 0,             147, format::button, 0                },
    { 0,             148, format::button, 0                },
    { 0,             149, format::button, 0                },
    { 0,             150, format::button, 0                },
    { 0,             151, format::button, 0                },
    { 0,             152, format::button, 0                },
    { 0,             153, format::button, 0                },
    { 0,             154, format::button, 0                },
    { 0,             155, format::button, 0                },
    { 0,             156, format::button, 0                },
    { 0,             157, format::button, 0                },
    { 0,             158, format::button, 0                },
    { 0,             159, format::button, 0                },
    { 0,             160, format::button, 0                },
    { 0,             161, format::button, 0                },
    { 0,             162, format::button, 0                },
    { 0,             163, format::button, 0                },
    { 0,             164, format::button, 0                },
    { 0,             165, format::button, 0                },
    { 0,             166, format::button, 0                },
    { 0,             167, format::button, 0                },
    { 0,             168, format::button, 0                },
    { 0,             169, format::button, 0                },
    { 0,             170, format::button, 0                },
    { 0,             171, format::button, 0                },
    { 0,             172, format::button, 0                },
    { 0,             173, format::button, 0                },
    { 0,             174, format::button, 0                },
    { 0,             175, format::button, 0                },
    { &x_axis_guid,  176, format::axis,   aspect::velocity },
    { &y_axis_guid,  180, format::axis,   aspect::velocity },
    { &z_axis_guid,  184, format::axis,   aspect::velocity },
    { &rx_axis_guid, 188, format::axis,   aspect::velocity },
    { &ry_axis_guid, 192, format::axis,   aspect::velocity },
    { &rz_axis_guid, 196, format::axis,   aspect::velocity },
    { &slider_guid,  200, format::axis,   aspect::velocity },
    { &slider_guid,  204, format::axis,   aspect::velocity },
    { &x_axis_guid,  208, format::axis,   aspect::accel    },
    { &y_axis_guid,  212, format::axis,   aspect::accel    },
    { &z_axis_guid,  216, format::axis,   aspect::accel    },
    { &rx_axis_guid, 220, format::axis,   aspect::accel    },
    { &ry_axis_guid, 224, format::axis,   aspect::accel    },
    { &rz_axis_guid, 228, format::axis,   aspect::accel    },
    { &slider_guid,  232, format::axis,   aspect::accel    },
    { &slider_guid,  236, format::axis,   aspect::accel    },
    { &x_axis_guid,  240, format::axis,   aspect::force    },
    { &y_axis_guid,  244, format::axis,   aspect::force    },
    { &z_axis_guid,  248, format::axis,   aspect::force    },
    { &rx_axis_guid, 252, format::axis,   aspect::force    },
    { &ry_axis_guid, 256, format::axis,   aspect::force    },
    { &rz_axis_guid, 260, format::axis,   aspect::force    },
    { &slider_guid,  264, format::axis,   aspect::force    },
    { &slider_guid,  268, format::axis,   aspect::force    }
};

} // namespace

extern const ::DIDATAFORMAT joystick_data_format =
{
    sizeof(::DIDATAFORMAT),
    sizeof(DIOBJECTDATAFORMAT),
    DIDF_ABSAXIS,
    sizeof(::DIJOYSTATE2),
    sizeof(joystick_obj_formats)/sizeof(joystick_obj_formats[0]),
    &joystick_obj_formats[0]

};

} } } // End namespaces direct_input, input, hamigaki.
