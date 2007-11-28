// new_stage_dialog.cpp: the dialog to input data for creating a new stage

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "new_stage_dialog.hpp"
#include <memory>
#include "new_dialog.h"

namespace
{

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

::INT_PTR CALLBACK new_dialog_proc(
    ::HWND hwndDlg, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        if (uMsg == WM_INITDIALOG)
        {
            stage_info* info = reinterpret_cast<stage_info*>(lParam);
            if (info)
            {
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
                std::auto_ptr<stage_info> info(new stage_info);
                if (get_dialog_item_int(
                        hwndDlg, HAMIGAKI_IDC_WIDTH, info->width) &&
                    get_dialog_item_int(
                        hwndDlg, HAMIGAKI_IDC_HEIGHT, info->height) )
                {
                    ::EndDialog(
                        hwndDlg, reinterpret_cast< ::INT_PTR>(info.release()));
                }
                return 1;
            }
            else if (id == IDCANCEL)
            {
                ::EndDialog(hwndDlg, 0);
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

bool get_new_stage_info(::HWND hwnd, stage_info& info)
{
    // FIXME
    ::HINSTANCE module =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::INT_PTR res = ::DialogBoxParamA(
        module, MAKEINTRESOURCE(HAMIGAKI_IDD_NEW),
        hwnd, &new_dialog_proc, reinterpret_cast< ::LPARAM>(&info)
    );

    if (res != 0)
    {
        std::auto_ptr<stage_info> result(reinterpret_cast<stage_info*>(res));
        info = *result;
        return true;
    }
    else
        return 0;
}
