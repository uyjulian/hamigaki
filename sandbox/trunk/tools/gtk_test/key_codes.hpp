// key_codes.hpp: the virtual key codes

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef HAMIGAKI_KEY_CODES_HPP
#define HAMIGAKI_KEY_CODES_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
    #include <windows.h>
    #define HAMIGAKI_KEY_CODE(w,x) ((int)(w))
#else
    #include <X11/keysym.h>
    #define HAMIGAKI_KEY_CODE(w,x) (x)
#endif

namespace hamigaki
{

const int key_back      = HAMIGAKI_KEY_CODE(VK_BACK,        XK_BackSpace);
const int key_tab       = HAMIGAKI_KEY_CODE(VK_TAB,         XK_Tab);

const int key_l_shift   = HAMIGAKI_KEY_CODE(VK_SHIFT,       XK_Shift_L);
const int key_l_ctrl    = HAMIGAKI_KEY_CODE(VK_CONTROL,     XK_Control_L);
const int key_l_alt     = HAMIGAKI_KEY_CODE(VK_MENU,        XK_Alt_L);
const int key_pause     = HAMIGAKI_KEY_CODE(VK_PAUSE,       XK_Pause);

const int key_escape    = HAMIGAKI_KEY_CODE(VK_ESCAPE,      XK_Escape);

const int key_space     = HAMIGAKI_KEY_CODE(VK_SPACE,       XK_space);

const int key_end       = HAMIGAKI_KEY_CODE(VK_END,         XK_End);
const int key_home      = HAMIGAKI_KEY_CODE(VK_HOME,        XK_Home);
const int key_left      = HAMIGAKI_KEY_CODE(VK_LEFT,        XK_Left);
const int key_up        = HAMIGAKI_KEY_CODE(VK_UP,          XK_Up);
const int key_right     = HAMIGAKI_KEY_CODE(VK_RIGHT,       XK_Right);
const int key_down      = HAMIGAKI_KEY_CODE(VK_DOWN,        XK_Down);
const int key_insert    = HAMIGAKI_KEY_CODE(VK_INSERT,      XK_Insert);
const int key_delete    = HAMIGAKI_KEY_CODE(VK_DELETE,      XK_Delete);

const int key_numpad0   = HAMIGAKI_KEY_CODE(VK_NUMPAD0,     XK_KP_0);
const int key_numpad1   = HAMIGAKI_KEY_CODE(VK_NUMPAD1,     XK_KP_1);
const int key_numpad2   = HAMIGAKI_KEY_CODE(VK_NUMPAD2,     XK_KP_2);
const int key_numpad3   = HAMIGAKI_KEY_CODE(VK_NUMPAD3,     XK_KP_3);
const int key_numpad4   = HAMIGAKI_KEY_CODE(VK_NUMPAD4,     XK_KP_4);
const int key_numpad5   = HAMIGAKI_KEY_CODE(VK_NUMPAD5,     XK_KP_5);
const int key_numpad6   = HAMIGAKI_KEY_CODE(VK_NUMPAD6,     XK_KP_6);
const int key_numpad7   = HAMIGAKI_KEY_CODE(VK_NUMPAD7,     XK_KP_7);
const int key_numpad8   = HAMIGAKI_KEY_CODE(VK_NUMPAD8,     XK_KP_8);
const int key_numpad9   = HAMIGAKI_KEY_CODE(VK_NUMPAD9,     XK_KP_9);
const int key_multiply  = HAMIGAKI_KEY_CODE(VK_MULTIPLY,    XK_KP_Multiply);
const int key_add       = HAMIGAKI_KEY_CODE(VK_ADD,         XK_KP_Add);
const int key_subtract  = HAMIGAKI_KEY_CODE(VK_SUBTRACT,    XK_KP_Subtract);
const int key_decimal   = HAMIGAKI_KEY_CODE(VK_DECIMAL,     XK_KP_Decimal);
const int key_divide    = HAMIGAKI_KEY_CODE(VK_DIVIDE,      XK_KP_Divide);
const int key_f1        = HAMIGAKI_KEY_CODE(VK_F1,          XK_F1);
const int key_f2        = HAMIGAKI_KEY_CODE(VK_F2,          XK_F2);
const int key_f3        = HAMIGAKI_KEY_CODE(VK_F3,          XK_F3);
const int key_f4        = HAMIGAKI_KEY_CODE(VK_F4,          XK_F4);
const int key_f5        = HAMIGAKI_KEY_CODE(VK_F5,          XK_F5);
const int key_f6        = HAMIGAKI_KEY_CODE(VK_F6,          XK_F6);
const int key_f7        = HAMIGAKI_KEY_CODE(VK_F7,          XK_F7);
const int key_f8        = HAMIGAKI_KEY_CODE(VK_F8,          XK_F8);
const int key_f9        = HAMIGAKI_KEY_CODE(VK_F9,          XK_F9);
const int key_f10       = HAMIGAKI_KEY_CODE(VK_F10,         XK_F10);
const int key_f11       = HAMIGAKI_KEY_CODE(VK_F11,         XK_F11);
const int key_f12       = HAMIGAKI_KEY_CODE(VK_F12,         XK_F12);

const int key_0         = HAMIGAKI_KEY_CODE('0',            XK_0);
const int key_1         = HAMIGAKI_KEY_CODE('1',            XK_1);
const int key_2         = HAMIGAKI_KEY_CODE('2',            XK_2);
const int key_3         = HAMIGAKI_KEY_CODE('3',            XK_3);
const int key_4         = HAMIGAKI_KEY_CODE('4',            XK_4);
const int key_5         = HAMIGAKI_KEY_CODE('5',            XK_5);
const int key_6         = HAMIGAKI_KEY_CODE('6',            XK_6);
const int key_7         = HAMIGAKI_KEY_CODE('7',            XK_7);
const int key_8         = HAMIGAKI_KEY_CODE('8',            XK_8);
const int key_9         = HAMIGAKI_KEY_CODE('9',            XK_9);

const int key_a         = HAMIGAKI_KEY_CODE('A',            XK_a);
const int key_b         = HAMIGAKI_KEY_CODE('B',            XK_b);
const int key_c         = HAMIGAKI_KEY_CODE('C',            XK_c);
const int key_d         = HAMIGAKI_KEY_CODE('D',            XK_d);
const int key_e         = HAMIGAKI_KEY_CODE('E',            XK_e);
const int key_f         = HAMIGAKI_KEY_CODE('F',            XK_f);
const int key_g         = HAMIGAKI_KEY_CODE('G',            XK_g);
const int key_h         = HAMIGAKI_KEY_CODE('H',            XK_h);
const int key_i         = HAMIGAKI_KEY_CODE('I',            XK_i);
const int key_j         = HAMIGAKI_KEY_CODE('J',            XK_j);
const int key_k         = HAMIGAKI_KEY_CODE('K',            XK_k);
const int key_l         = HAMIGAKI_KEY_CODE('L',            XK_l);
const int key_m         = HAMIGAKI_KEY_CODE('M',            XK_m);
const int key_n         = HAMIGAKI_KEY_CODE('N',            XK_n);
const int key_o         = HAMIGAKI_KEY_CODE('O',            XK_o);
const int key_p         = HAMIGAKI_KEY_CODE('P',            XK_p);
const int key_q         = HAMIGAKI_KEY_CODE('Q',            XK_q);
const int key_r         = HAMIGAKI_KEY_CODE('R',            XK_r);
const int key_s         = HAMIGAKI_KEY_CODE('S',            XK_s);
const int key_t         = HAMIGAKI_KEY_CODE('T',            XK_t);
const int key_u         = HAMIGAKI_KEY_CODE('U',            XK_u);
const int key_v         = HAMIGAKI_KEY_CODE('V',            XK_v);
const int key_w         = HAMIGAKI_KEY_CODE('W',            XK_w);
const int key_x         = HAMIGAKI_KEY_CODE('X',            XK_x);
const int key_y         = HAMIGAKI_KEY_CODE('Y',            XK_y);
const int key_z         = HAMIGAKI_KEY_CODE('Z',            XK_z);

const int key_r_shift   = HAMIGAKI_KEY_CODE(VK_RSHIFT,      XK_Shift_R);
const int key_r_ctrl    = HAMIGAKI_KEY_CODE(VK_RCONTROL,    XK_Control_R);
const int key_r_alt     = HAMIGAKI_KEY_CODE(VK_RMENU,       XK_Alt_R);

} // namespace hamigaki

#undef HAMIGAKI_KEY_CODE

#endif // HAMIGAKI_KEY_CODES_HPP
