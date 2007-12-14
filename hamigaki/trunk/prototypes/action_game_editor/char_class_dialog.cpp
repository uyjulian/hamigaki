// char_class_dialog.cpp: the dialog to input data for editing characters

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "char_class_dialog.hpp"
#include "icon_select_dialog.hpp"
#include <boost/lexical_cast.hpp>
#include <exception>
#include <cstring>
#include "char_dialog.h"

namespace
{

const ::GUID move_routines[] =
{
    {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0x461C9BC7,0x803A,0x41D6,{0x95,0xC2,0x06,0x35,0xEB,0x4C,0xC5,0x7C}},
    {0x2DF594A8,0x470B,0x4125,{0xAD,0xE3,0x0F,0xDA,0x70,0xA2,0xAD,0x1D}}
};

const char* move_routine_names[] =
{
    "None",
    "Velocity-based",
    "Loop Lift"
};

const ::GUID speed_routines[] =
{
    {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0x352F6C96,0x62A6,0x4C9A,{0x92,0x2E,0x84,0xD4,0x5F,0xCF,0xB9,0x0C}},
    {0x453DC7FE,0x0039,0x4F5E,{0x93,0x37,0x88,0x66,0x83,0x4D,0x4B,0x49}},
    {0xA67E5469,0xC71E,0x4A48,{0xA5,0xE3,0x5C,0x7E,0x79,0x6E,0x72,0x82}}
};

const char* speed_routine_names[] =
{
    "None",
    "Turn",
    "Hop",
    "Hop-step-jump"
};

const ::GUID collision_routines[] =
{
    {0x00000000,0x0000,0x0000,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0x6A199951,0x2B1C,0x45BB,{0x9E,0xC0,0xFF,0x55,0x29,0x96,0xE5,0xA3}},
    {0xBAD301D1,0x2A1C,0x4156,{0xA1,0x25,0x35,0x74,0x6D,0xAC,0x48,0x83}},
    {0xBD31F66D,0x891B,0x4ED4,{0xAE,0xC1,0x32,0x7C,0xFF,0x2C,0x01,0xBC}},
    {0x52CDB853,0xE6F3,0x48CA,{0xA1,0x2A,0x9D,0xE6,0xE7,0xF6,0xE7,0x6B}},
    {0x122AB3E8,0xC2E5,0x429F,{0x80,0x88,0x0C,0x8B,0x7B,0x71,0x07,0xBB}},
    {0x3CB2EDAF,0x4AF1,0x498B,{0x80,0x25,0x95,0x27,0xA8,0xB1,0x38,0x08}},
    {0xC2209D0A,0x811E,0x4F31,{0x80,0x66,0x24,0xF4,0xCD,0x99,0x82,0x96}},
    {0x796CFF8F,0x1DB7,0x4D80,{0xB7,0xBD,0xAC,0x2E,0xD1,0x2E,0xCC,0xA0}}
};

const char* collision_routine_names[] =
{
    "None",
    "Hit",
    "Pop-up Item",
    "Power Down",
    "Secret Coin",
    "Stomp",
    "To Fragments",
    "To Used Block",
    "Turn"
};

inline void set_dialog_item_text(::HWND hwnd, int id, const std::string& s)
{
    ::SetDlgItemTextA(hwnd, id, s.c_str());
}

inline std::string get_dialog_item_text(::HWND hwnd, int id)
{
    char buf[256];
    int n = ::GetDlgItemTextA(hwnd, id, buf, sizeof(buf));
    return std::string(buf, n);
}

inline void add_dialog_item_string(::HWND hwnd, int id, const char* s)
{
    ::SendDlgItemMessage(
        hwnd, id, CB_ADDSTRING, 0,
        reinterpret_cast< ::LPARAM>(s)
    );
}

template<std::size_t Size>
void set_dialog_item_strings(::HWND hwnd, int id, const char* (&items)[Size])
{
    for (std::size_t i = 0; i < Size; ++i)
        add_dialog_item_string(hwnd, id, items[i]);
}

template<std::size_t Size>
std::size_t find_guid(const GUID (&items)[Size], const hamigaki::uuid& id)
{
    ::GUID buf;
    id.copy(buf);

    for (std::size_t i = 0; i < Size; ++i)
        if (std::memcmp(&items[i], &buf, sizeof(GUID)) == 0)
            return i;

    return 0;
}

void setup_collision_event(::HWND hwnd, int id, const hamigaki::uuid& type)
{
    set_dialog_item_strings(hwnd, id, collision_routine_names);

    ::SendDlgItemMessage(
        hwnd, id, CB_SETCURSEL, find_guid(collision_routines, type), 0
    );
}

hamigaki::uuid get_collision_event(::HWND hwnd, int id)
{
    int index = ::SendDlgItemMessage(hwnd, id, CB_GETCURSEL, 0, 0);
    return hamigaki::uuid(collision_routines[index]);
}

::INT_PTR CALLBACK char_dialog_proc(
    ::HWND hwndDlg, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        if (uMsg == WM_INITDIALOG)
        {
            game_character_class* info =
                reinterpret_cast<game_character_class*>(lParam);
            if (info)
            {
                ::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

                set_dialog_item_text(
                    hwndDlg, HAMIGAKI_IDC_SPRITE, info->sprite);

                ::CheckDlgButton(
                    hwndDlg, HAMIGAKI_IDC_IS_PLAYER,
                    info->attrs.test(char_attr::player));

                ::CheckDlgButton(
                    hwndDlg, HAMIGAKI_IDC_IS_ENEMY,
                    info->attrs.test(char_attr::enemy));

                ::CheckDlgButton(
                    hwndDlg, HAMIGAKI_IDC_IS_WEAPON,
                    info->attrs.test(char_attr::weapon));

                ::CheckDlgButton(
                    hwndDlg, HAMIGAKI_IDC_IS_BLOCK,
                    info->attrs.test(char_attr::block));

                ::CheckDlgButton(
                    hwndDlg, HAMIGAKI_IDC_IS_BREAKER,
                    info->attrs.test(char_attr::breaker));

                set_dialog_item_text(
                    hwndDlg, HAMIGAKI_IDC_VX,
                    boost::lexical_cast<std::string>(info->vx));

                set_dialog_item_text(
                    hwndDlg, HAMIGAKI_IDC_VY,
                    boost::lexical_cast<std::string>(info->vy));

                add_dialog_item_string(hwndDlg, HAMIGAKI_IDC_SLOPE, "None");

                add_dialog_item_string(
                    hwndDlg, HAMIGAKI_IDC_SLOPE, "Left-down");

                add_dialog_item_string(
                    hwndDlg, HAMIGAKI_IDC_SLOPE, "Right-down");

                ::SendDlgItemMessage(
                    hwndDlg, HAMIGAKI_IDC_SLOPE, CB_SETCURSEL,
                    static_cast<int>(info->slope), 0
                );

                set_dialog_item_strings(
                    hwndDlg, HAMIGAKI_IDC_ON_MOVE, move_routine_names);

                ::SendDlgItemMessage(
                    hwndDlg, HAMIGAKI_IDC_ON_MOVE, CB_SETCURSEL,
                    find_guid(move_routines, info->move_routine), 0
                );

                set_dialog_item_strings(
                    hwndDlg, HAMIGAKI_IDC_ON_SPEED, speed_routine_names);

                ::SendDlgItemMessage(
                    hwndDlg, HAMIGAKI_IDC_ON_SPEED, CB_SETCURSEL,
                    find_guid(speed_routines, info->speed_routine), 0
                );

                setup_collision_event(
                    hwndDlg, HAMIGAKI_IDC_ON_BLOCK, info->on_collide_block_side
                );

                setup_collision_event(
                    hwndDlg, HAMIGAKI_IDC_ON_PUSH, info->on_hit_from_below
                );

                setup_collision_event(
                    hwndDlg, HAMIGAKI_IDC_ON_PLAYER, info->on_collide_player
                );

                setup_collision_event(
                    hwndDlg, HAMIGAKI_IDC_ON_ENEMY, info->on_collide_enemy
                );

                setup_collision_event(
                    hwndDlg, HAMIGAKI_IDC_ON_STOMP, info->on_stomp
                );

                setup_collision_event(
                    hwndDlg, HAMIGAKI_IDC_ON_HIT, info->on_hit
                );
            }
            return 1;
        }
        else if (uMsg == WM_COMMAND)
        {
            ::WORD id = LOWORD(wParam);
            if (id == IDOK)
            {
                game_character_class* info =
                    reinterpret_cast<game_character_class*>(
                        ::GetWindowLongPtr(hwndDlg, DWLP_USER)
                    );

                info->sprite =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_SPRITE);

                info->attrs.set(
                    char_attr::player,
                    ::IsDlgButtonChecked(hwndDlg, HAMIGAKI_IDC_IS_PLAYER) != 0
                );

                info->attrs.set(
                    char_attr::enemy,
                    ::IsDlgButtonChecked(hwndDlg, HAMIGAKI_IDC_IS_ENEMY) != 0
                );

                info->attrs.set(
                    char_attr::weapon,
                    ::IsDlgButtonChecked(hwndDlg, HAMIGAKI_IDC_IS_WEAPON) != 0
                );

                info->attrs.set(
                    char_attr::block,
                    ::IsDlgButtonChecked(hwndDlg, HAMIGAKI_IDC_IS_BLOCK) != 0
                );

                info->attrs.set(
                    char_attr::breaker,
                    ::IsDlgButtonChecked(hwndDlg, HAMIGAKI_IDC_IS_BREAKER) != 0
                );

                info->vx = boost::lexical_cast<float>(
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_VX)
                );

                info->vy = boost::lexical_cast<float>(
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_VY)
                );

                info->slope = static_cast<slope_type::values>(
                    ::SendDlgItemMessage(
                        hwndDlg, HAMIGAKI_IDC_SLOPE, CB_GETCURSEL, 0, 0)
                );

                int on_move = ::SendDlgItemMessage(
                        hwndDlg, HAMIGAKI_IDC_ON_MOVE, CB_GETCURSEL, 0, 0);
                info->move_routine = hamigaki::uuid(move_routines[on_move]);

                int on_speed = ::SendDlgItemMessage(
                        hwndDlg, HAMIGAKI_IDC_ON_SPEED, CB_GETCURSEL, 0, 0);
                info->speed_routine = hamigaki::uuid(speed_routines[on_speed]);

                info->on_collide_block_side =
                    get_collision_event(hwndDlg, HAMIGAKI_IDC_ON_BLOCK);
                info->on_hit_from_below =
                    get_collision_event(hwndDlg, HAMIGAKI_IDC_ON_PUSH);
                info->on_collide_player =
                    get_collision_event(hwndDlg, HAMIGAKI_IDC_ON_PLAYER);
                info->on_collide_enemy =
                    get_collision_event(hwndDlg, HAMIGAKI_IDC_ON_ENEMY);
                info->on_stomp =
                    get_collision_event(hwndDlg, HAMIGAKI_IDC_ON_STOMP);
                info->on_hit =
                    get_collision_event(hwndDlg, HAMIGAKI_IDC_ON_HIT);

                ::EndDialog(hwndDlg, IDOK);
                return 1;
            }
            else if (id == IDCANCEL)
            {
                ::EndDialog(hwndDlg, IDCANCEL);
                return 1;
            }
            else if (id == HAMIGAKI_IDC_SPRITE_BTN) // FIXME
            {
                icon_info info;
                info.filename = "char_chips.png";
                info.x = 1;
                info.y = 2;
                select_icon(hwndDlg, info);
            }
        }
        else
            return 0;
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwndDlg, e.what(), "Action Game Editor", MB_OK);
    }
    return 0;
}

} // namespace

bool get_character_class_info(::HWND hwnd, game_character_class& info)
{
    // FIXME
    ::HINSTANCE module =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::INT_PTR res = ::DialogBoxParamA(
        module, MAKEINTRESOURCE(HAMIGAKI_IDD_CHAR),
        hwnd, &char_dialog_proc, reinterpret_cast< ::LPARAM>(&info)
    );

    return res == IDOK;
}
