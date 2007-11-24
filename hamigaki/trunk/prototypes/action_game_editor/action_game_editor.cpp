// action_game_editor.cpp: a map editor for action_game.exe

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window.hpp"
#include <iostream>
#include <stdexcept>

#include <windows.h>
#include <commctrl.h>

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

int WINAPI WinMain(
    ::HINSTANCE hInstance, ::HINSTANCE, ::LPSTR, int nCmdShow)
{
    try
    {
        init_common_controls();

        ::ATOM cls = register_main_window_class(hInstance);
        ::HWND hwnd = create_main_window(hInstance, cls);
        connect_d3d_device(hwnd);

        ::ShowWindow(hwnd, nCmdShow);
        ::UpdateWindow(hwnd);

        ::MSG msg;
        msg.message = WM_NULL;
        while (msg.message != WM_QUIT)
        {
            ::BOOL res = ::GetMessageA(&msg, 0, 0, 0);
            if (res == -1)
                break;

            if (res != 0)
            {
                ::TranslateMessage(&msg);
                ::DispatchMessageA(&msg);
            }
        }

        ::UnregisterClassA(MAKEINTATOM(cls), hInstance);
        return msg.wParam;
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(0, e.what(), "Action Game Editor", MB_OK);
    }
    return 0;
}
