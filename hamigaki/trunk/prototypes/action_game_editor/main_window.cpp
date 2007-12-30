// main_window.cpp: main window for action_game_editor

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window.hpp"
#include "char_select_window_msgs.hpp"
#include "main_window_impl.hpp"
#include "map_config_dialog.hpp"
#include "msg_utilities.hpp"
#include "project_config_dialog.hpp"
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <cstring>

#include <hamigaki/system/windows_error.hpp>
#include <shlobj.h>
#include "menus.h"
#include "popup_menus.h"

namespace fs = boost::filesystem;
using hamigaki::system::windows_error;

namespace
{

std::string get_documents_path()
{
    char buf[MAX_PATH];
    if (::SHGetSpecialFolderPathA(0, buf, CSIDL_PERSONAL, FALSE) == FALSE)
        throw std::runtime_error("failed SHGetSpecialFolderPath()");
    return buf;
}

void enable_project_menu(::HWND hwnd, bool value)
{
    ::HMENU menu = ::GetMenu(hwnd);

    ::DWORD flags = value ? MF_ENABLED : MF_GRAYED;

    ::EnableMenuItem(menu, HAMIGAKI_ID_FILE_CLOSE, flags);
    ::EnableMenuItem(menu, HAMIGAKI_ID_FILE_SAVE,  flags);
}

bool get_open_file_name(::HWND hwnd, std::string& filename)
{
    char buf[MAX_PATH];
    buf[0] = '\0';

    ::OPENFILENAMEA ofn;
    std::memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter =
        "Action Game Project Files (*.agp-yh)\0*.agp-yh\0"
        ;
    ofn.Flags = OFN_EXPLORER|OFN_FILEMUSTEXIST;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = sizeof(buf);

    if (::GetOpenFileNameA(&ofn) == FALSE)
        return false;

    filename = buf;
    return true;
}

bool close_project(main_window* pimpl, ::HWND hwnd)
{
    if (pimpl->modified())
    {
        int res = ::MessageBoxA(hwnd,
            "The current project is modified. Do you save it?",
            "Action Game Editor", MB_YESNOCANCEL|MB_ICONEXCLAMATION);

        if (res == IDCANCEL)
            return false;
        else if (res == IDYES)
            pimpl->save_project();
    }

    pimpl->close_project();
    enable_project_menu(hwnd, false);
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
            if (close_project(pimpl, hwnd))
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
        else if (uMsg == WM_CONTEXTMENU)
        {
            ::HWND handle = reinterpret_cast< ::HWND>(wParam);
            int x = static_cast<short>(LOWORD(lParam));
            int y = static_cast<short>(HIWORD(lParam));
            if (pimpl)
                pimpl->track_popup_menu(handle, x, y);
        }
        else if (uMsg == WM_COMMAND)
        {
            ::WORD code = HIWORD(wParam);
            ::WORD id = LOWORD(wParam);
            if (id == HAMIGAKI_ID_FILE_NEW)
            {
                if (!close_project(pimpl, hwnd))
                    return 0;

                fs::path base = fs::path(get_documents_path()) / "ActionGame";
                fs::path dir;
                for (int i = 1; ; ++i)
                {
                    dir = base / (boost::format("Project%d") % i).str();
                    if (!fs::exists(dir))
                        break;
                }

                game_project proj;
                proj.dir = dir.directory_string();
                if (get_project_info(hwnd, proj))
                {
                    fs::create_directories(fs::path(proj.dir));
                    fs::path ph = fs::path(proj.dir) / "action_game.agp-yh";
                    pimpl->new_project(ph.file_string(), proj);
                    enable_project_menu(hwnd, true);
                }
            }
            else if (id == HAMIGAKI_ID_FILE_OPEN)
            {
                if (!close_project(pimpl, hwnd))
                    return 0;

                std::string filename;
                if (get_open_file_name(hwnd, filename))
                {
                    pimpl->load_project(filename);
                    enable_project_menu(hwnd, true);
                }
            }
            else if (id == HAMIGAKI_ID_FILE_CLOSE)
                close_project(pimpl, hwnd);
            else if (id == HAMIGAKI_ID_FILE_SAVE)
                pimpl->save_project();
            else if (id == HAMIGAKI_ID_FILE_EXIT)
            {
                if (close_project(pimpl, hwnd))
                    ::DestroyWindow(hwnd);
            }
            else if (id == HAMIGAKI_ID_EDIT_DEL)
            {
                ::HWND focus = ::GetFocus();
                if (focus == ::GetDlgItem(hwnd, main_window::map_select_id))
                    send_command(hwnd, HAMIGAKI_ID_MAP_DEL, 1, 0);
            }
            else if (id == HAMIGAKI_ID_MAP_NEW)
            {
                stage_info info;
                info.name =
                    (boost::format("map%03d") % (pimpl->stage_count()+1)).str();
                info.width = 20;
                info.height = 15;

                while (get_stage_info(hwnd, info))
                {
                    const std::string& filename = info.name + ".agm-yh";
                    if (pimpl->new_stage(filename, info.width, info.height))
                        break;
                }
            }
            else if (id == HAMIGAKI_ID_MAP_DEL)
            {
                std::string msg;
                msg += "Do you delete '";
                msg += pimpl->stage_name();
                msg += "' ?";

                int res = ::MessageBoxA(hwnd,
                    msg.c_str(),
                    "Action Game Editor", MB_YESNO|MB_ICONINFORMATION);

                if (res == IDYES)
                    pimpl->delete_stage();
            }
            else if (id == HAMIGAKI_ID_GAME_PROP)
            {
                std::vector<std::string> map_names;
                pimpl->get_stage_names(map_names);

                game_project proj = pimpl->project_info();
                if (get_project_info(hwnd, proj, map_names))
                    pimpl->project_info(proj);
            }
            else if (id == main_window::char_select_id)
            {
                if (code == char_select_window_msgs::notify_sel_changed)
                    pimpl->update_selected_char();
            }
            else if (id == main_window::map_select_id)
            {
                if (code == LBN_SELCHANGE)
                    pimpl->change_stage();
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

::HWND create_main_window(::HINSTANCE hInstance)
{
    ::DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;
    ::DWORD ex_style = 0;

    ::HWND hwnd = ::CreateWindowExA(
        ex_style, "MainWindow", "Action Game Editor", style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, hInstance, 0
    );
    if (hwnd == 0)
        throw windows_error(::GetLastError(), "CreateWindowExA()");

    return hwnd;
}
