// icon_select_dialog.cpp: the dialog to select an icon

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "icon_select_dialog.hpp"
#include "icon_select_window.hpp"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/scoped_array.hpp>
#include <exception>
#include "icon_dialog.h"

namespace algo = boost::algorithm;
namespace fs = boost::filesystem;

namespace
{

std::string get_window_text(::HWND hwnd)
{
    int len = ::GetWindowTextLength(hwnd);
    boost::scoped_array<char> buf(new char[len+1]);
    ::GetWindowTextA(hwnd, buf.get(), len+1);
    return std::string(buf.get());
}

void update_icons(::HWND hwndDlg)
{
    ::HWND hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_PNG);
    const std::string& filename = get_window_text(hwnd);

    select_window_load(
        ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_ICON_SEL),
        filename
    );
}

void setup_filename_list(::HWND hwndDlg, const std::string& filename)
{
    ::HWND hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_PNG);

    fs::directory_iterator it((fs::current_path()));
    fs::directory_iterator end;

    std::locale loc("");
    for ( ; it != end; ++it)
    {
        const std::string& leaf = it->path().leaf();
        if (algo::iends_with(leaf, ".png", loc))
        {
            ::SendMessageA(
                hwnd, CB_ADDSTRING, 0,
                reinterpret_cast< ::LPARAM>(leaf.c_str())
            );
        }
    }

    if (!filename.empty())
    {
        int index = ::SendMessageA(
            hwnd, CB_FINDSTRINGEXACT,
            0, reinterpret_cast< ::LPARAM>(filename.c_str())
        );

        if (index != CB_ERR)
        {
            ::SendMessageA(hwnd, CB_SETCURSEL, index, 0);
            update_icons(hwndDlg);
        }
    }
}

::INT_PTR CALLBACK icon_dialog_proc(
    ::HWND hwndDlg, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        ::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
        icon_info* info = reinterpret_cast<icon_info*>(lParam);

        setup_filename_list(hwndDlg, info->filename);
        select_window_cursor_pos(
            ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_ICON_SEL), info->x, info->y);

        return 1;
    }
    else if (uMsg == WM_COMMAND)
    {
        ::WORD code = HIWORD(wParam);
        ::WORD id = LOWORD(wParam);
        if ((code == 0) || (code == 1))
        {
            if (id == IDOK)
            {
                icon_info* info =
                    reinterpret_cast<icon_info*>(
                        GetWindowLongPtr(hwndDlg, DWLP_USER)
                    );

                ::HWND hwnd = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_PNG);
                info->filename = get_window_text(hwnd);

                ::HWND hwnd2 = ::GetDlgItem(hwndDlg, HAMIGAKI_IDC_ICON_SEL);
                const std::pair<int,int>& x_y = select_window_cursor_pos(hwnd2);
                info->x = x_y.first;
                info->y = x_y.second;

                ::EndDialog(hwndDlg, IDOK);
                return 1;
            }
            else if (id == IDCANCEL)
            {
                ::EndDialog(hwndDlg, IDCANCEL);
                return 1;
            }
        }
        else if (id == HAMIGAKI_IDC_PNG)
        {
            if (code == CBN_SELENDOK)
                update_icons(hwndDlg);
        }
    }
    return 0;
}

} // namespace

bool select_icon(::HWND hwnd, icon_info& info)
{
    // FIXME
    ::HINSTANCE module =
        reinterpret_cast< ::HINSTANCE>(::GetModuleHandle(0));

    ::INT_PTR res = ::DialogBoxParamA(
        module, MAKEINTRESOURCE(HAMIGAKI_IDD_ICON),
        hwnd, &icon_dialog_proc, reinterpret_cast< ::LPARAM>(&info)
    );

    return res == IDOK;
}
