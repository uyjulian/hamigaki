// action_game.cpp: an action game of the prototype

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window.hpp"
#include <hamigaki/system/windows_error.hpp>
#include <exception>
#include <iostream>

#include <windows.h>

using hamigaki::system::windows_error;

int WINAPI WinMain(
    ::HINSTANCE hInstance, ::HINSTANCE, ::LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        ::ATOM cls = register_main_window_class(hInstance);
        ::HWND hwnd = create_main_window(hInstance, cls);

        ::ShowWindow(hwnd, nCmdShow);
        ::UpdateWindow(hwnd);
        connect_d3d_device(hwnd);

        ::MSG msg;
        msg.message = WM_NULL;
        while (msg.message != WM_QUIT)
        {
            if (::PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessageA(&msg);
            }
            else
            {
                process_input(hwnd);
                render(hwnd);
            }
        }

        ::UnregisterClassA(MAKEINTATOM(cls), hInstance);
        return msg.wParam;
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(0, e.what(), "Action Game", MB_OK);
    }
    return 0;
}
