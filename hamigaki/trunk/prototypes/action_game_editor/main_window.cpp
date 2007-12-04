// main_window.cpp: main window for action_game_editor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window.hpp"
#include "char_select_window_msgs.hpp"
#include "main_window_impl.hpp"
#include "new_stage_dialog.hpp"
#include <boost/algorithm/string/find.hpp>
#include <cstring>

#include <hamigaki/system/windows_error.hpp>
#include "menus.h"

using hamigaki::system::windows_error;

namespace
{

bool get_open_file_name(::HWND hwnd, std::string& filename)
{
    char buf[MAX_PATH];
    buf[0] = '\0';

    ::OPENFILENAMEA ofn;
    std::memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter =
        "Text Files (*.txt)\0*.txt\0"
        ;
    ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = sizeof(buf);

    if (::GetOpenFileNameA(&ofn) == FALSE)
        return false;

    filename = buf;
    return true;
}

::UINT_PTR CALLBACK
save_dialog_hook(::HWND hdlg, ::UINT uiMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    if (uiMsg == WM_NOTIFY)
    {
        ::NMHDR* head = reinterpret_cast< ::NMHDR*>(lParam);
        if (head->code == CDN_TYPECHANGE)
        {
            ::OPENFILENAMEA& info =
                *reinterpret_cast< ::OFNOTIFYA*>(head)->lpOFN;

            if (info.nFilterIndex == 1)
                CommDlg_OpenSave_SetDefExt(hdlg, "agm-yh");
            else if (info.nFilterIndex == 2)
                CommDlg_OpenSave_SetDefExt(hdlg, "txt");
        }
    }
    return 0;
}

bool get_save_file_name(::HWND hwnd, std::string& filename)
{
    char buf[MAX_PATH];
    buf[0] = '\0';

    ::OPENFILENAMEA ofn;
    std::memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter =
        "Action Game Map Files (*.agm-yh)\0*.agm-yh\0"
        "Text Files (*.txt)\0*.txt\0"
        ;
    ofn.Flags = OFN_EXPLORER|OFN_ENABLEHOOK|OFN_OVERWRITEPROMPT;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = sizeof(buf);
    ofn.lpstrDefExt = "agm-yh";
    ofn.lpfnHook = &save_dialog_hook;

    if (::GetSaveFileName(&ofn) == FALSE)
        return false;

    filename = buf;
    return true;
}

bool save_stage(main_window* pimpl, ::HWND hwnd)
{
    if (pimpl->save_stage())
        return true;

    std::string filename;
    if (get_save_file_name(hwnd, filename))
    {
        pimpl->save_stage(filename);
        return true;
    }
    else
        return false;
}

bool close_stage(main_window* pimpl, ::HWND hwnd)
{
    if (pimpl->modified())
    {
        int res = ::MessageBoxA(hwnd,
            "The current stage is modified. Do you save it?",
            "Action Game Editor", MB_YESNOCANCEL|MB_ICONEXCLAMATION);

        if (res == IDCANCEL)
            return false;
        else if (res == IDYES)
        {
            if (!save_stage(pimpl, hwnd))
                return false;
        }
    }

    return true;
}

::LRESULT CALLBACK window_proc(
    ::HWND hwnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        main_window* pimpl =
            reinterpret_cast<main_window*>(
                GetWindowLongPtr(hwnd, GWLP_USERDATA)
            );

        if (uMsg == WM_GETMINMAXINFO)
        {
            ::MINMAXINFO& info = *reinterpret_cast< ::MINMAXINFO*>(lParam);
            info.ptMinTrackSize.x = 200;
            info.ptMinTrackSize.y = 400;
            return 0;
        }
        else if (uMsg == WM_CREATE)
        {
            pimpl = new main_window(hwnd);
            SetWindowLongPtr(
                hwnd, GWLP_USERDATA, reinterpret_cast< ::LONG_PTR>(pimpl));
        }
        else if (uMsg == WM_DESTROY)
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            delete pimpl;
            ::PostQuitMessage(0);
        }
        else if (uMsg == WM_CLOSE)
        {
            if (close_stage(pimpl, hwnd))
                ::DestroyWindow(hwnd);
            return 0;
        }
        else if (uMsg == WM_SIZE)
        {
            if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED))
            {
                if (pimpl)
                    pimpl->update_size();
            }
        }
        else if (uMsg == WM_COMMAND)
        {
            ::WORD code = HIWORD(wParam);
            ::WORD id = LOWORD(wParam);
            if ((code == 0) || (code == 1))
            {
                if (id == HAMIGAKI_ID_FILE_NEW)
                {
                    if (!close_stage(pimpl, hwnd))
                        return 0;

                    stage_info info;
                    if (get_new_stage_info(hwnd, info))
                        pimpl->new_stage(info.width, info.height);
                }
                else if (id == HAMIGAKI_ID_FILE_OPEN)
                {
                    if (!close_stage(pimpl, hwnd))
                        return 0;

                    std::string filename;
                    if (get_open_file_name(hwnd, filename))
                        pimpl->load_stage(filename);
                }
                else if (id == HAMIGAKI_ID_FILE_SAVE)
                    save_stage(pimpl, hwnd);
                else if (id == HAMIGAKI_ID_FILE_SAVE_AS)
                {
                    std::string filename;
                    if (get_save_file_name(hwnd, filename))
                        pimpl->save_stage(filename);
                }
                else if (id == HAMIGAKI_ID_FILE_EXIT)
                {
                    if (close_stage(pimpl, hwnd))
                        ::DestroyWindow(hwnd);
                }
            }
            else if (id == main_window::char_select_id)
            {
                if (code == char_select_window_msgs::notify_sel_changed)
                    pimpl->update_selected_char();
            }
        }
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwnd, e.what(), "Action Game Editor", MB_OK);
    }
    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

} // namespace

::ATOM register_main_window_class(::HINSTANCE hInstance)
{
    ::WNDCLASSEXA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(::LONG_PTR);
    wc.hInstance = hInstance;
    wc.hIcon = ::LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = ::LoadCursor(0, IDC_ARROW);
    wc.hbrBackground =
        reinterpret_cast< ::HBRUSH>(
            static_cast< ::INT_PTR>(COLOR_BTNFACE+1)
        );
    wc.lpszMenuName = MAKEINTRESOURCE(HAMIGAKI_IDR_MENU);
    wc.lpszClassName = "MainWindow";
    wc.hIconSm = 0;

    ::ATOM cls = ::RegisterClassExA(&wc);
    if (cls == 0)
        throw windows_error(::GetLastError(), "RegisterClassExA()");

    return cls;
}

::HWND create_main_window(::HINSTANCE hInstance, ::ATOM cls)
{
    ::DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    ::DWORD ex_style = 0;

    ::HWND hwnd = ::CreateWindowExA(
        ex_style, MAKEINTATOM(cls), "Action Game Editor", style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, hInstance, 0
    );
    if (hwnd == 0)
        throw windows_error(::GetLastError(), "CreateWindowExA()");

    return hwnd;
}
