// transfer_dialog.cpp: the dialog to input info for setting a transfer

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "transfer_dialog.hpp"
#include "msg_utilities.hpp"
#include "position_dialog.hpp"
#include <locale>
#include <sstream>

#include "trans_dialog.h"

namespace
{

std::string make_position_string(const transfer_info& info)
{
    std::ostringstream os;
    os.imbue(std::locale::classic());
    os << info.map_file << " (" << info.x << ", " << info.y << ')';
    return os.str();
}

void setup_directions(::HWND hwnd)
{
    send_msg(hwnd, CB_ADDSTRING, 0, "None");
    send_msg(hwnd, CB_ADDSTRING, 0, "Left");
    send_msg(hwnd, CB_ADDSTRING, 0, "Right");
    send_msg(hwnd, CB_ADDSTRING, 0, "Down");
    send_msg(hwnd, CB_ADDSTRING, 0, "Up");
}

void set_direction(::HWND hwnd, transfer_info::direction dir)
{
    send_msg(hwnd, CB_SETCURSEL, static_cast<int>(dir));
}

transfer_info::direction get_direction(::HWND hwnd)
{
    int index = send_msg(hwnd, CB_GETCURSEL);

    if (index != -1)
        return static_cast<transfer_info::direction>(index);
    else
        return transfer_info::none;
}

::INT_PTR CALLBACK trans_dialog_proc(
    ::HWND hwndDlg, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        transfer_info_params* params;
        if (uMsg == WM_INITDIALOG)
        {
            params = reinterpret_cast<transfer_info_params*>(lParam);
            ::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
        }
        else
        {
            params = reinterpret_cast<transfer_info_params*>(
                ::GetWindowLongPtr(hwndDlg, DWLP_USER)
            );
        }
        transfer_info& info = params->info;

        if (uMsg == WM_INITDIALOG)
        {
            ::SetDlgItemTextA(
                hwndDlg, HAMIGAKI_IDC_DEST,
                make_position_string(info).c_str()
            );

            ::HWND enter_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_ENTER);
            setup_directions(enter_hwnd);
            set_direction(enter_hwnd, info.enter_dir);

            ::HWND leave_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_LEAVE);
            setup_directions(leave_hwnd);
            set_direction(leave_hwnd, info.leave_dir);

            return 1;
        }
        else if (uMsg == WM_COMMAND)
        {
            ::WORD id = LOWORD(wParam);
            ::WORD code = HIWORD(wParam);
            if (id == IDOK)
            {
                ::HWND enter_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_ENTER);
                info.enter_dir = get_direction(enter_hwnd);

                ::HWND leave_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_LEAVE);
                info.leave_dir = get_direction(leave_hwnd);

                ::EndDialog(hwndDlg, IDOK);
                return 1;
            }
            else if (id == IDCANCEL)
            {
                ::EndDialog(hwndDlg, IDCANCEL);
                return 1;
            }
            else if (id == HAMIGAKI_IDC_DEST_SEL)
            {
                map_position_info tmp;
                tmp.map_table = params->map_table;
                tmp.chars = params->chars;
                tmp.bg_color = params->bg_color;
                tmp.filename = info.map_file;
                tmp.x = info.x;
                tmp.y = info.y;

                if (get_map_position(hwndDlg, tmp))
                {
                    info.map_file = tmp.filename;
                    info.x = tmp.x;
                    info.y = tmp.y;

                    ::SetDlgItemTextA(
                        hwndDlg, HAMIGAKI_IDC_DEST,
                        make_position_string(info).c_str()
                    );
                }
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

bool get_transfer_info(::HWND hwnd, transfer_info_params& params)
{
    // FIXME
    ::HINSTANCE module =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::INT_PTR res = ::DialogBoxParamA(
        module, MAKEINTRESOURCE(HAMIGAKI_IDD_TRANS),
        hwnd, &trans_dialog_proc, reinterpret_cast< ::LPARAM>(&params)
    );

    return (res == IDOK);
}
