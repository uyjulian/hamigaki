// map_config_dialog.cpp: the dialog to input data for setting a map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "map_config_dialog.hpp"
#include <exception>
#include "map_cfg_dialog.h"

namespace
{

std::string get_dialog_item_text(::HWND hwnd, int id)
{
    char buf[256];
    ::GetDlgItemTextA(hwnd, id, buf, sizeof(buf));
    return std::string(buf);
}

inline void set_dialog_item_int(::HWND hwnd, int id, int value)
{
    ::SetDlgItemInt(hwnd, id, static_cast<unsigned>(value), FALSE);
}

inline bool get_dialog_item_int(::HWND hwnd, int id, int& value)
{
    ::BOOL res;
    value = static_cast<int>(::GetDlgItemInt(hwnd, id, &res, FALSE));
    return res != FALSE;
}

::INT_PTR CALLBACK map_cfg_dialog_proc(
    ::HWND hwndDlg, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        if (uMsg == WM_INITDIALOG)
        {
            stage_info* info = reinterpret_cast<stage_info*>(lParam);
            if (info)
            {
                ::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
                ::SetDlgItemTextA(
                    hwndDlg, HAMIGAKI_IDC_MAP_NAME, info->name.c_str());
                set_dialog_item_int(hwndDlg, HAMIGAKI_IDC_WIDTH, info->width);
                set_dialog_item_int(hwndDlg, HAMIGAKI_IDC_HEIGHT, info->height);
            }
            return 1;
        }
        else if (uMsg == WM_COMMAND)
        {
            ::WORD id = LOWORD(wParam);
            if (id == IDOK)
            {
                stage_info* info =
                    reinterpret_cast<stage_info*>(
                        ::GetWindowLongPtr(hwndDlg, DWLP_USER)
                    );

                info->name =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_MAP_NAME);

                if (!info->name.empty() &&
                    get_dialog_item_int(
                        hwndDlg, HAMIGAKI_IDC_WIDTH, info->width) &&
                    get_dialog_item_int(
                        hwndDlg, HAMIGAKI_IDC_HEIGHT, info->height) )
                {
                    ::EndDialog(hwndDlg, IDOK);
                }
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

bool get_stage_info(::HWND hwnd, stage_info& info)
{
    // FIXME
    ::HINSTANCE module =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::INT_PTR res = ::DialogBoxParamA(
        module, MAKEINTRESOURCE(HAMIGAKI_IDD_MAP_CFG),
        hwnd, &map_cfg_dialog_proc, reinterpret_cast< ::LPARAM>(&info)
    );

    return res == IDOK;
}
