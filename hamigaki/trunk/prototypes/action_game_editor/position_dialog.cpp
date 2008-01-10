// position_dialog.cpp: the dialog to input info for setting a project

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "position_dialog.hpp"
#include "position_select_window.hpp"
#include "msg_utilities.hpp"
#include "pos_dialog.h"
#include <boost/tuple/tuple.hpp>
#include <boost/scoped_array.hpp>

namespace
{

void setup_map_list(
    ::HWND hwnd, const std::map<std::string,stage_map>& map_table)
{
    send_msg(hwnd, LB_RESETCONTENT);

    typedef std::map<std::string,stage_map>::const_iterator iter_type;
    for (iter_type i = map_table.begin(), end = map_table.end(); i != end; ++i)
        send_msg(hwnd, LB_ADDSTRING, 0, i->first);

    send_msg(hwnd, LB_SETCURSEL, 0);
}

std::string get_selected_string(::HWND hwnd)
{
    int index = send_msg(hwnd, LB_GETCURSEL);
    if (index == -1)
        return std::string();

    int size = send_msg(hwnd, LB_GETTEXTLEN, index);

    boost::scoped_array<char> buf(new char[size+1]);
    send_msg_with_ptr(hwnd, LB_GETTEXT, index, buf.get());

    return std::string(buf.get(), size);
}

::INT_PTR CALLBACK pos_dialog_proc(
    ::HWND hwndDlg, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        map_position_info* info;
        if (uMsg == WM_INITDIALOG)
        {
            info = reinterpret_cast<map_position_info*>(lParam);
            ::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
        }
        else
        {
            info = reinterpret_cast<map_position_info*>(
                ::GetWindowLongPtr(hwndDlg, DWLP_USER)
            );
        }

        if (uMsg == WM_INITDIALOG)
        {
            ::HWND sel_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_MAP_SEL);
            setup_map_list(sel_hwnd, *info->map_table);

            ::HWND map_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_MAP);

            typedef std::map<std::string,stage_map>::iterator iter_type;
            iter_type beg = info->map_table->begin();
            info->filename = beg->first;

            position_select_window_set(map_hwnd, &beg->second);
            position_select_window_set_char_list(map_hwnd, info->chars);
            position_select_window_set_bg_color(map_hwnd, info->bg_color);

            return 1;
        }
        else if (uMsg == WM_COMMAND)
        {
            ::WORD id = LOWORD(wParam);
            ::WORD code = HIWORD(wParam);
            if (id == IDOK)
            {
                ::HWND sel_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_MAP_SEL);
                info->filename = get_selected_string(sel_hwnd);

                ::HWND map_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_MAP);
                boost::tie(info->x, info->y) =
                    position_select_window_cursor_pos(map_hwnd);

                ::EndDialog(hwndDlg, IDOK);
                return 1;
            }
            else if (id == IDCANCEL)
            {
                ::EndDialog(hwndDlg, IDCANCEL);
                return 1;
            }
            else if (id == HAMIGAKI_IDC_MAP_SEL)
            {
                if (code == LBN_SELCHANGE)
                {
                    ::HWND sel_hwnd =
                        ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_MAP_SEL);
                    const std::string& name = get_selected_string(sel_hwnd);

                    ::HWND map_hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_MAP);
                    if (name.empty())
                        position_select_window_set(map_hwnd, 0);
                    else
                    {
                        position_select_window_set(
                            map_hwnd, &(*info->map_table)[name]);
                    }
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

bool get_map_position(::HWND hwnd, map_position_info& info)
{
    // FIXME
    ::HINSTANCE module =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::INT_PTR res = ::DialogBoxParamA(
        module, MAKEINTRESOURCE(HAMIGAKI_IDD_POS),
        hwnd, &pos_dialog_proc, reinterpret_cast< ::LPARAM>(&info)
    );

    return (res == IDOK);
}
