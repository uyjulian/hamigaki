// project_config_dialog.cpp: the dialog to input data for setting a project

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "project_config_dialog.hpp"
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <exception>
#include "proj_cfg_dialog.h"

namespace fs = boost::filesystem;

namespace
{

inline void set_dialog_item_text(::HWND hwnd, int id, const std::string& s)
{
    ::SetDlgItemTextA(hwnd, id, s.c_str());
}

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

::INT_PTR CALLBACK proj_cfg_dialog_proc(
    ::HWND hwndDlg, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        if (uMsg == WM_INITDIALOG)
        {
            game_project* info = reinterpret_cast<game_project*>(lParam);
            if (info)
            {
                ::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
                std::string name = fs::path(info->dir).leaf();
                set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_FOLDER, name);
                if (!info->title.empty())
                {
                    set_dialog_item_text(
                        hwndDlg, HAMIGAKI_IDC_TITLE, info->title);
                }
                else
                    set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_TITLE, name);
                set_dialog_item_text(hwndDlg, HAMIGAKI_IDC_DIR, info->dir);
                set_dialog_item_int(
                    hwndDlg, HAMIGAKI_IDC_SCREEN_W, info->screen_width);
                set_dialog_item_int(
                    hwndDlg, HAMIGAKI_IDC_SCREEN_H, info->screen_height);
                set_dialog_item_text(
                    hwndDlg, HAMIGAKI_IDC_GRAVITY,
                    boost::lexical_cast<std::string>(info->gravity));
                set_dialog_item_text(
                    hwndDlg, HAMIGAKI_IDC_MIN_VY,
                    boost::lexical_cast<std::string>(info->min_vy));
            }
            return 1;
        }
        else if (uMsg == WM_COMMAND)
        {
            ::WORD id = LOWORD(wParam);
            if (id == IDOK)
            {
                game_project* info =
                    reinterpret_cast<game_project*>(
                        ::GetWindowLongPtr(hwndDlg, DWLP_USER)
                    );

                info->title =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_TITLE);
                info->dir =
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_DIR);
                info->gravity = boost::lexical_cast<float>(
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_GRAVITY)
                );
                info->min_vy = boost::lexical_cast<float>(
                    get_dialog_item_text(hwndDlg, HAMIGAKI_IDC_MIN_VY)
                );

                if (!info->dir.empty() &&
                    get_dialog_item_int(
                        hwndDlg, HAMIGAKI_IDC_SCREEN_W, info->screen_width) &&
                    get_dialog_item_int(
                        hwndDlg, HAMIGAKI_IDC_SCREEN_H, info->screen_height) )
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

bool get_project_info(::HWND hwnd, game_project& info)
{
    // FIXME
    ::HINSTANCE module =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::INT_PTR res = ::DialogBoxParamA(
        module, MAKEINTRESOURCE(HAMIGAKI_IDD_PROJ_CFG),
        hwnd, &proj_cfg_dialog_proc, reinterpret_cast< ::LPARAM>(&info)
    );

    return res == IDOK;
}
