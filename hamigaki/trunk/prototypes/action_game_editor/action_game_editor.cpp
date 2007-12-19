// action_game_editor.cpp: a map editor for action_game.exe

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "icon_select_window.hpp"
#include "icon_view_window.hpp"
#include "main_window.hpp"
#include <boost/noncopyable.hpp>
#include <iostream>
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
    ::HINSTANCE hInstance, ::HINSTANCE, ::LPSTR, int nCmdShow)
{
    try
    {
        const hamigaki::detail::windows::com_library using_com;
        init_common_controls();

        accelerator_table accels(hInstance, HAMIGAKI_IDR_ACCELERATOR);

        ::ATOM cls = register_main_window_class(hInstance);
        ::ATOM cls2 = register_icon_select_window_class(hInstance);
        ::ATOM cls3 = register_icon_view_window_class(hInstance);
        ::HWND hwnd = create_main_window(hInstance, cls);

        ::ShowWindow(hwnd, nCmdShow);
        ::UpdateWindow(hwnd);

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

        ::UnregisterClassA(MAKEINTATOM(cls3), hInstance);
        ::UnregisterClassA(MAKEINTATOM(cls2), hInstance);
        ::UnregisterClassA(MAKEINTATOM(cls), hInstance);
        return msg.wParam;
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(0, e.what(), "Action Game Editor", MB_OK);
    }
    return 0;
}
