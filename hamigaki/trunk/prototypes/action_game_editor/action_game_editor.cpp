// action_game_editor.cpp: a map editor for action_game.exe

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "char_select_window.hpp"
#include "icon_select_window.hpp"
#include "icon_view_window.hpp"
#include "main_window.hpp"
#include "map_edit_window.hpp"
#include "position_select_window.hpp"
#include <boost/noncopyable.hpp>
#include <cstring>
#include <locale>
#include <stdexcept>

#include <hamigaki/detail/windows/com_library.hpp>
#include <windows.h>
#include <commctrl.h>

#include "accelerators.h"

void init_common_controls()
{
#if defined(__MINGW32__)
    ::InitCommonControls();
#else
    ::INITCOMMONCONTROLSEX data;
    std::memset(&data, 0, sizeof(data));
    data.dwSize = sizeof(data);
    data.dwICC = ICC_BAR_CLASSES;
    if (::InitCommonControlsEx(&data) == FALSE)
        throw std::runtime_error("InitCommonControlsEx() failed");
#endif
}

class accelerator_table : private boost::noncopyable
{
public:
    accelerator_table(::HINSTANCE hInstance, int id)
        : handle_(::LoadAcceleratorsA(hInstance, MAKEINTRESOURCE(id)))
    {
    }

    ~accelerator_table()
    {
        ::DestroyAcceleratorTable(handle_);
    }

    bool process_message(::HWND hwnd, ::MSG& msg)
    {
        return ::TranslateAcceleratorA(hwnd, handle_, &msg) != 0;
    }

private:
    ::HACCEL handle_;
};

int WINAPI WinMain(
    ::HINSTANCE hInstance, ::HINSTANCE, ::LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        std::locale::global(std::locale(""));

        const hamigaki::detail::windows::com_library using_com;
        init_common_controls();

        accelerator_table accels(hInstance, HAMIGAKI_IDR_ACCELERATOR);

        register_char_select_window_class(hInstance);
        register_icon_select_window_class(hInstance);
        register_icon_view_window_class(hInstance);
        register_main_window_class(hInstance);
        register_map_edit_window_class(hInstance);
        register_position_select_window_class(hInstance);

        ::HWND hwnd = create_main_window(hInstance);

        ::ShowWindow(hwnd, nCmdShow);
        ::UpdateWindow(hwnd);

        if (*lpCmdLine)
            main_window_load_project(hwnd, lpCmdLine);

        ::MSG msg;
        ::BOOL res;
        while ((res = ::GetMessageA(&msg, 0, 0, 0)) != 0)
        {
            if (res == -1)
            {
                ::MessageBoxA(
                    hwnd, "failed GetMessageA()", "Action Game Editor", MB_OK);
                break;
            }

            if (!accels.process_message(hwnd, msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessageA(&msg);
            }
        }
        return msg.wParam;
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(0, e.what(), "Action Game Editor", MB_OK);
    }
    return 0;
}
