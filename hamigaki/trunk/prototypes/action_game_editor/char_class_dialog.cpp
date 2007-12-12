// char_class_dialog.cpp: the dialog to input data for editing characters

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "char_class_dialog.hpp"
#include <exception>
#include "char_dialog.h"

namespace
{

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

                ::EndDialog(hwndDlg, IDOK);
                return 1;
            }
            else if (id == IDCANCEL)
            {
                ::EndDialog(hwndDlg, IDCANCEL);
                return 1;
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
