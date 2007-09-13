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

const ::GUID key_guid =
{ 0x55728220,0xD33C,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

const ::GUID pov_guid =
{ 0xA36D02F2,0xC9F3,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };


struct format
{
    static const ::DWORD axis   = DIDFT_AXIS  |DIDFT_ANYINSTANCE|DIDFT_OPTIONAL;
    static const ::DWORD pov    = DIDFT_POV   |DIDFT_ANYINSTANCE|DIDFT_OPTIONAL;
    static const ::DWORD button = DIDFT_BUTTON|DIDFT_ANYINSTANCE|DIDFT_OPTIONAL;

    template< ::WORD N>
    struct key_button
    {
        static const ::DWORD value =
            DIDFT_BUTTON|(static_cast< ::DWORD>(N)<<8)|DIDFT_OPTIONAL;
    };
};

struct aspect
{
    static const ::DWORD position = DIDOI_ASPECTPOSITION;
    static const ::DWORD velocity = DIDOI_ASPECTVELOCITY;
    static const ::DWORD accel    = DIDOI_ASPECTACCEL;
    static const ::DWORD force    = DIDOI_ASPECTFORCE;
};

::DIOBJECTDATAFORMAT keyboard_obj_formats[] =
{ 
    { &key_guid,   0, format::key_button<  0>::value, 0 },
    { &key_guid,   1, format::key_button<  1>::value, 0 },
    { &key_guid,   2, format::key_button<  2>::value, 0 },
    { &key_guid,   3, format::key_button<  3>::value, 0 },
    { &key_guid,   4, format::key_button<  4>::value, 0 },
    { &key_guid,   5, format::key_button<  5>::value, 0 },
    { &key_guid,   6, format::key_button<  6>::value, 0 },
    { &key_guid,   7, format::key_button<  7>::value, 0 },
    { &key_guid,   8, format::key_button<  8>::value, 0 },
    { &key_guid,   9, format::key_button<  9>::value, 0 },
    { &key_guid,  10, format::key_button< 10>::value, 0 },
    { &key_guid,  11, format::key_button< 11>::value, 0 },
    { &key_guid,  12, format::key_button< 12>::value, 0 },
    { &key_guid,  13, format::key_button< 13>::value, 0 },
    { &key_guid,  14, format::key_button< 14>::value, 0 },
    { &key_guid,  15, format::key_button< 15>::value, 0 },
    { &key_guid,  16, format::key_button< 16>::value, 0 },
    { &key_guid,  17, format::key_button< 17>::value, 0 },
    { &key_guid,  18, format::key_button< 18>::value, 0 },
    { &key_guid,  19, format::key_button< 19>::value, 0 },
    { &key_guid,  20, format::key_button< 20>::value, 0 },
    { &key_guid,  21, format::key_button< 21>::value, 0 },
    { &key_guid,  22, format::key_button< 22>::value, 0 },
    { &key_guid,  23, format::key_button< 23>::value, 0 },
    { &key_guid,  24, format::key_button< 24>::value, 0 },
    { &key_guid,  25, format::key_button< 25>::value, 0 },
    { &key_guid,  26, format::key_button< 26>::value, 0 },
    { &key_guid,  27, format::key_button< 27>::value, 0 },
    { &key_guid,  28, format::key_button< 28>::value, 0 },
    { &key_guid,  29, format::key_button< 29>::value, 0 },
    { &key_guid,  30, format::key_button< 30>::value, 0 },
    { &key_guid,  31, format::key_button< 31>::value, 0 },
    { &key_guid,  32, format::key_button< 32>::value, 0 },
    { &key_guid,  33, format::key_button< 33>::value, 0 },
    { &key_guid,  34, format::key_button< 34>::value, 0 },
    { &key_guid,  35, format::key_button< 35>::value, 0 },
    { &key_guid,  36, format::key_button< 36>::value, 0 },
    { &key_guid,  37, format::key_button< 37>::value, 0 },
    { &key_guid,  38, format::key_button< 38>::value, 0 },
    { &key_guid,  39, format::key_button< 39>::value, 0 },
    { &key_guid,  40, format::key_button< 40>::value, 0 },
    { &key_guid,  41, format::key_button< 41>::value, 0 },
    { &key_guid,  42, format::key_button< 42>::value, 0 },
    { &key_guid,  43, format::key_button< 43>::value, 0 },
    { &key_guid,  44, format::key_button< 44>::value, 0 },
    { &key_guid,  45, format::key_button< 45>::value, 0 },
    { &key_guid,  46, format::key_button< 46>::value, 0 },
    { &key_guid,  47, format::key_button< 47>::value, 0 },
    { &key_guid,  48, format::key_button< 48>::value, 0 },
    { &key_guid,  49, format::key_button< 49>::value, 0 },
    { &key_guid,  50, format::key_button< 50>::value, 0 },
    { &key_guid,  51, format::key_button< 51>::value, 0 },
    { &key_guid,  52, format::key_button< 52>::value, 0 },
    { &key_guid,  53, format::key_button< 53>::value, 0 },
    { &key_guid,  54, format::key_button< 54>::value, 0 },
    { &key_guid,  55, format::key_button< 55>::value, 0 },
    { &key_guid,  56, format::key_button< 56>::value, 0 },
    { &key_guid,  57, format::key_button< 57>::value, 0 },
    { &key_guid,  58, format::key_button< 58>::value, 0 },
    { &key_guid,  59, format::key_button< 59>::value, 0 },
    { &key_guid,  60, format::key_button< 60>::value, 0 },
    { &key_guid,  61, format::key_button< 61>::value, 0 },
    { &key_guid,  62, format::key_button< 62>::value, 0 },
    { &key_guid,  63, format::key_button< 63>::value, 0 },
    { &key_guid,  64, format::key_button< 64>::value, 0 },
    { &key_guid,  65, format::key_button< 65>::value, 0 },
    { &key_guid,  66, format::key_button< 66>::value, 0 },
    { &key_guid,  67, format::key_button< 67>::value, 0 },
    { &key_guid,  68, format::key_button< 68>::value, 0 },
    { &key_guid,  69, format::key_button< 69>::value, 0 },
    { &key_guid,  70, format::key_button< 70>::value, 0 },
    { &key_guid,  71, format::key_button< 71>::value, 0 },
    { &key_guid,  72, format::key_button< 72>::value, 0 },
    { &key_guid,  73, format::key_button< 73>::value, 0 },
    { &key_guid,  74, format::key_button< 74>::value, 0 },
    { &key_guid,  75, format::key_button< 75>::value, 0 },
    { &key_guid,  76, format::key_button< 76>::value, 0 },
    { &key_guid,  77, format::key_button< 77>::value, 0 },
    { &key_guid,  78, format::key_button< 78>::value, 0 },
    { &key_guid,  79, format::key_button< 79>::value, 0 },
    { &key_guid,  80, format::key_button< 80>::value, 0 },
    { &key_guid,  81, format::key_button< 81>::value, 0 },
    { &key_guid,  82, format::key_button< 82>::value, 0 },
    { &key_guid,  83, format::key_button< 83>::value, 0 },
    { &key_guid,  84, format::key_button< 84>::value, 0 },
    { &key_guid,  85, format::key_button< 85>::value, 0 },
    { &key_guid,  86, format::key_button< 86>::value, 0 },
    { &key_guid,  87, format::key_button< 87>::value, 0 },
    { &key_guid,  88, format::key_button< 88>::value, 0 },
    { &key_guid,  89, format::key_button< 89>::value, 0 },
    { &key_guid,  90, format::key_button< 90>::value, 0 },
    { &key_guid,  91, format::key_button< 91>::value, 0 },
    { &key_guid,  92, format::key_button< 92>::value, 0 },
    { &key_guid,  93, format::key_button< 93>::value, 0 },
    { &key_guid,  94, format::key_button< 94>::value, 0 },
    { &key_guid,  95, format::key_button< 95>::value, 0 },
    { &key_guid,  96, format::key_button< 96>::value, 0 },
    { &key_guid,  97, format::key_button< 97>::value, 0 },
    { &key_guid,  98, format::key_button< 98>::value, 0 },
    { &key_guid,  99, format::key_button< 99>::value, 0 },
    { &key_guid, 100, format::key_button<100>::value, 0 },
    { &key_guid, 101, format::key_button<101>::value, 0 },
    { &key_guid, 102, format::key_button<102>::value, 0 },
    { &key_guid, 103, format::key_button<103>::value, 0 },
    { &key_guid, 104, format::key_button<104>::value, 0 },
    { &key_guid, 105, format::key_button<105>::value, 0 },
    { &key_guid, 106, format::key_button<106>::value, 0 },
    { &key_guid, 107, format::key_button<107>::value, 0 },
    { &key_guid, 108, format::key_button<108>::value, 0 },
    { &key_guid, 109, format::key_button<109>::value, 0 },
    { &key_guid, 110, format::key_button<110>::value, 0 },
    { &key_guid, 111, format::key_button<111>::value, 0 },
    { &key_guid, 112, format::key_button<112>::value, 0 },
    { &key_guid, 113, format::key_button<113>::value, 0 },
    { &key_guid, 114, format::key_button<114>::value, 0 },
    { &key_guid, 115, format::key_button<115>::value, 0 },
    { &key_guid, 116, format::key_button<116>::value, 0 },
    { &key_guid, 117, format::key_button<117>::value, 0 },
    { &key_guid, 118, format::key_button<118>::value, 0 },
    { &key_guid, 119, format::key_button<119>::value, 0 },
    { &key_guid, 120, format::key_button<120>::value, 0 },
    { &key_guid, 121, format::key_button<121>::value, 0 },
    { &key_guid, 122, format::key_button<122>::value, 0 },
    { &key_guid, 123, format::key_button<123>::value, 0 },
    { &key_guid, 124, format::key_button<124>::value, 0 },
    { &key_guid, 125, format::key_button<125>::value, 0 },
    { &key_guid, 126, format::key_button<126>::value, 0 },
    { &key_guid, 127, format::key_button<127>::value, 0 },
    { &key_guid, 128, format::key_button<128>::value, 0 },
    { &key_guid, 129, format::key_button<129>::value, 0 },
    { &key_guid, 130, format::key_button<130>::value, 0 },
    { &key_guid, 131, format::key_button<131>::value, 0 },
    { &key_guid, 132, format::key_button<132>::value, 0 },
    { &key_guid, 133, format::key_button<133>::value, 0 },
    { &key_guid, 134, format::key_button<134>::value, 0 },
    { &key_guid, 135, format::key_button<135>::value, 0 },
    { &key_guid, 136, format::key_button<136>::value, 0 },
    { &key_guid, 137, format::key_button<137>::value, 0 },
    { &key_guid, 138, format::key_button<138>::value, 0 },
    { &key_guid, 139, format::key_button<139>::value, 0 },
    { &key_guid, 140, format::key_button<140>::value, 0 },
    { &key_guid, 141, format::key_button<141>::value, 0 },
    { &key_guid, 142, format::key_button<142>::value, 0 },
    { &key_guid, 143, format::key_button<143>::value, 0 },
    { &key_guid, 144, format::key_button<144>::value, 0 },
    { &key_guid, 145, format::key_button<145>::value, 0 },
    { &key_guid, 146, format::key_button<146>::value, 0 },
    { &key_guid, 147, format::key_button<147>::value, 0 },
    { &key_guid, 148, format::key_button<148>::value, 0 },
    { &key_guid, 149, format::key_button<149>::value, 0 },
    { &key_guid, 150, format::key_button<150>::value, 0 },
    { &key_guid, 151, format::key_button<151>::value, 0 },
    { &key_guid, 152, format::key_button<152>::value, 0 },
    { &key_guid, 153, format::key_button<153>::value, 0 },
    { &key_guid, 154, format::key_button<154>::value, 0 },
    { &key_guid, 155, format::key_button<155>::value, 0 },
    { &key_guid, 156, format::key_button<156>::value, 0 },
    { &key_guid, 157, format::key_button<157>::value, 0 },
    { &key_guid, 158, format::key_button<158>::value, 0 },
    { &key_guid, 159, format::key_button<159>::value, 0 },
    { &key_guid, 160, format::key_button<160>::value, 0 },
    { &key_guid, 161, format::key_button<161>::value, 0 },
    { &key_guid, 162, format::key_button<162>::value, 0 },
    { &key_guid, 163, format::key_button<163>::value, 0 },
    { &key_guid, 164, format::key_button<164>::value, 0 },
    { &key_guid, 165, format::key_button<165>::value, 0 },
    { &key_guid, 166, format::key_button<166>::value, 0 },
    { &key_guid, 167, format::key_button<167>::value, 0 },
    { &key_guid, 168, format::key_button<168>::value, 0 },
    { &key_guid, 169, format::key_button<169>::value, 0 },
    { &key_guid, 170, format::key_button<170>::value, 0 },
    { &key_guid, 171, format::key_button<171>::value, 0 },
    { &key_guid, 172, format::key_button<172>::value, 0 },
    { &key_guid, 173, format::key_button<173>::value, 0 },
    { &key_guid, 174, format::key_button<174>::value, 0 },
    { &key_guid, 175, format::key_button<175>::value, 0 },
    { &key_guid, 176, format::key_button<176>::value, 0 },
    { &key_guid, 177, format::key_button<177>::value, 0 },
    { &key_guid, 178, format::key_button<178>::value, 0 },
    { &key_guid, 179, format::key_button<179>::value, 0 },
    { &key_guid, 180, format::key_button<180>::value, 0 },
    { &key_guid, 181, format::key_button<181>::value, 0 },
    { &key_guid, 182, format::key_button<182>::value, 0 },
    { &key_guid, 183, format::key_button<183>::value, 0 },
    { &key_guid, 184, format::key_button<184>::value, 0 },
    { &key_guid, 185, format::key_button<185>::value, 0 },
    { &key_guid, 186, format::key_button<186>::value, 0 },
    { &key_guid, 187, format::key_button<187>::value, 0 },
    { &key_guid, 188, format::key_button<188>::value, 0 },
    { &key_guid, 189, format::key_button<189>::value, 0 },
    { &key_guid, 190, format::key_button<190>::value, 0 },
    { &key_guid, 191, format::key_button<191>::value, 0 },
    { &key_guid, 192, format::key_button<192>::value, 0 },
    { &key_guid, 193, format::key_button<193>::value, 0 },
    { &key_guid, 194, format::key_button<194>::value, 0 },
    { &key_guid, 195, format::key_button<195>::value, 0 },
    { &key_guid, 196, format::key_button<196>::value, 0 },
    { &key_guid, 197, format::key_button<197>::value, 0 },
    { &key_guid, 198, format::key_button<198>::value, 0 },
    { &key_guid, 199, format::key_button<199>::value, 0 },
    { &key_guid, 200, format::key_button<200>::value, 0 },
    { &key_guid, 201, format::key_button<201>::value, 0 },
    { &key_guid, 202, format::key_button<202>::value, 0 },
    { &key_guid, 203, format::key_button<203>::value, 0 },
    { &key_guid, 204, format::key_button<204>::value, 0 },
    { &key_guid, 205, format::key_button<205>::value, 0 },
    { &key_guid, 206, format::key_button<206>::value, 0 },
    { &key_guid, 207, format::key_button<207>::value, 0 },
    { &key_guid, 208, format::key_button<208>::value, 0 },
    { &key_guid, 209, format::key_button<209>::value, 0 },
    { &key_guid, 210, format::key_button<210>::value, 0 },
    { &key_guid, 211, format::key_button<211>::value, 0 },
    { &key_guid, 212, format::key_button<212>::value, 0 },
    { &key_guid, 213, format::key_button<213>::value, 0 },
    { &key_guid, 214, format::key_button<214>::value, 0 },
    { &key_guid, 215, format::key_button<215>::value, 0 },
    { &key_guid, 216, format::key_button<216>::value, 0 },
    { &key_guid, 217, format::key_button<217>::value, 0 },
    { &key_guid, 218, format::key_button<218>::value, 0 },
    { &key_guid, 219, format::key_button<219>::value, 0 },
    { &key_guid, 220, format::key_button<220>::value, 0 },
    { &key_guid, 221, format::key_button<221>::value, 0 },
    { &key_guid, 222, format::key_button<222>::value, 0 },
    { &key_guid, 223, format::key_button<223>::value, 0 },
    { &key_guid, 224, format::key_button<224>::value, 0 },
    { &key_guid, 225, format::key_button<225>::value, 0 },
    { &key_guid, 226, format::key_button<226>::value, 0 },
    { &key_guid, 227, format::key_button<227>::value, 0 },
    { &key_guid, 228, format::key_button<228>::value, 0 },
    { &key_guid, 229, format::key_button<229>::value, 0 },
    { &key_guid, 230, format::key_button<230>::value, 0 },
    { &key_guid, 231, format::key_button<231>::value, 0 },
    { &key_guid, 232, format::key_button<232>::value, 0 },
    { &key_guid, 233, format::key_button<233>::value, 0 },
    { &key_guid, 234, format::key_button<234>::value, 0 },
    { &key_guid, 235, format::key_button<235>::value, 0 },
    { &key_guid, 236, format::key_button<236>::value, 0 },
    { &key_guid, 237, format::key_button<237>::value, 0 },
    { &key_guid, 238, format::key_button<238>::value, 0 },
    { &key_guid, 239, format::key_button<239>::value, 0 },
    { &key_guid, 240, format::key_button<240>::value, 0 },
    { &key_guid, 241, format::key_button<241>::value, 0 },
    { &key_guid, 242, format::key_button<242>::value, 0 },
    { &key_guid, 243, format::key_button<243>::value, 0 },
    { &key_guid, 244, format::key_button<244>::value, 0 },
    { &key_guid, 245, format::key_button<245>::value, 0 },
    { &key_guid, 246, format::key_button<246>::value, 0 },
    { &key_guid, 247, format::key_button<247>::value, 0 },
    { &key_guid, 248, format::key_button<248>::value, 0 },
    { &key_guid, 249, format::key_button<249>::value, 0 },
    { &key_guid, 250, format::key_button<250>::value, 0 },
    { &key_guid, 251, format::key_button<251>::value, 0 },
    { &key_guid, 252, format::key_button<252>::value, 0 },
    { &key_guid, 253, format::key_button<253>::value, 0 },
    { &key_guid, 254, format::key_button<254>::value, 0 },
    { &key_guid, 255, format::key_button<255>::value, 0 },
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

extern const ::DIDATAFORMAT keyboard_data_format =
{
    sizeof(::DIDATAFORMAT),
    sizeof(::DIOBJECTDATAFORMAT),
    DIDF_RELAXIS,
    sizeof(keyboard_obj_formats)/sizeof(keyboard_obj_formats[0]),
    sizeof(keyboard_obj_formats)/sizeof(keyboard_obj_formats[0]),
    &keyboard_obj_formats[0]
};

extern const ::DIDATAFORMAT joystick_data_format =
{
    sizeof(::DIDATAFORMAT),
    sizeof(::DIOBJECTDATAFORMAT),
    DIDF_ABSAXIS,
    sizeof(::DIJOYSTATE2),
    sizeof(joystick_obj_formats)/sizeof(joystick_obj_formats[0]),
    &joystick_obj_formats[0]
};

} } } // End namespaces direct_input, input, hamigaki.
