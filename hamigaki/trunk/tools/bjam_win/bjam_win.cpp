// bjam_win.cpp: a GUI front-end for bjam

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <stdexcept>

#include "main_window.hpp"
#include <windows.h>
#include <commctrl.h>
#include "menus.h"

void init_common_controls()
{
#if defined(__MINGW32__)
    ::InitCommonControls();
#else
    ::INITCOMMONCONTROLSEX data;
    std::memset(&data, 0, sizeof(data));
    data.dwSize = sizeof(data);
    data.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES;
    if (::InitCommonControlsEx(&data) == FALSE)
        throw std::runtime_error("InitCommonControlsEx() failed");
#endif
}

int WINAPI WinMain(
    ::HINSTANCE hInstance, ::HINSTANCE, ::LPSTR lpCmdLine, int nCmdShow)
{
    try
    {
        init_common_controls();

        ::ATOM cls = register_main_window_class(hInstance);
        ::HWND hwnd = create_main_window(hInstance, cls);

        ::ShowWindow(hwnd, nCmdShow);
        ::UpdateWindow(hwnd);

        if (*lpCmdLine != '\0')
        {
            std::string filename = lpCmdLine;

            if ((filename.size() >= 2) &&
                (filename[0] == '"') &&
                (filename[filename.size()-1] == '"') )
            {
                filename.erase(0, 1);
                filename.erase(filename.size()-1, 1);
            }

            main_window_open_jamfile(hwnd, filename);
        }

        ::MSG msg;
        while (::GetMessage(&msg, 0, 0, 0))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }

        ::UnregisterClass(MAKEINTATOM(cls), hInstance);
        return msg.wParam;
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(0, e.what(), "bjam for Windows", MB_OK);
    }
    return 0;
}
