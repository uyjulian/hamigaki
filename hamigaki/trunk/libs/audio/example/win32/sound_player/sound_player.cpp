//  sound_player.cpp: a GUI example for background_player

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <cstring>
#include <stdexcept>

#include "main_window.hpp"
#include <windows.h>
#include <commctrl.h>
#include "controls.h"
#include "menus.h"

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

bool get_open_file_name(::HWND hwnd, std::string& filename)
{
    char buf[MAX_PATH];
    buf[0] = '\0';

    ::OPENFILENAMEA ofn;
    std::memset(&ofn, 0, sizeof(ofn));
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
    ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400A;
#else
    ofn.lStructSize = sizeof(ofn);
#endif
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter =
        "WAV Files (*.wav)\0*.wav\0"
        "Ogg Files (*.ogg)\0*.ogg\0"
        ;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = sizeof(buf);

    if (::GetOpenFileNameA(&ofn) == FALSE)
        return false;

    filename = buf;
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

        if (uMsg == WM_CREATE)
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
        else if (uMsg == WM_COMMAND)
        {
            ::WORD code = LOWORD(wParam);
            if (code == IDC_PLAY)
            {
                if (pimpl->playing())
                    pimpl->pause();
                else
                    pimpl->play();
            }
            else if (code == IDC_STOP)
                pimpl->stop();
            else if (code == ID_FILE_LOAD)
            {
                std::string filename;
                if (::get_open_file_name(hwnd, filename))
                {
                    pimpl->open(filename);
                    pimpl->play();
                }
            }
            else if (code == ID_FILE_EXIT)
                ::DestroyWindow(hwnd);
        }
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(0, e.what(), "Sound Player", MB_OK);
    }
    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

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
        reinterpret_cast< ::HBRUSH>(static_cast< ::INT_PTR>(COLOR_WINDOW+1));
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
    wc.lpszClassName = "MainWindow";
    wc.hIconSm = 0;

    ::ATOM cls = ::RegisterClassExA(&wc);
    if (cls == 0)
        throw std::runtime_error("RegisterClassExA() failed");

    return cls;
}

::HWND create_main_window(::HINSTANCE hInstance, ::ATOM cls)
{
    ::RECT r;
    r.left = 0;
    r.top = 0;
    r.right = 280;
    r.bottom = 100;

    ::DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    ::DWORD ex_style = 0;

    ::AdjustWindowRectEx(&r, style, TRUE, ex_style);

    ::HWND hwnd = ::CreateWindowEx(
        ex_style, MAKEINTATOM(cls), "Sound Player", style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        r.right-r.left, r.bottom-r.top, 0, 0, hInstance, 0
    );
    if (hwnd == 0)
        throw std::runtime_error("CreateWindowEx() failed");

    return hwnd;
}

int WINAPI WinMain(::HINSTANCE hInstance, ::HINSTANCE, ::LPSTR, int nCmdShow)
{
    try
    {
        init_common_controls();

        ::ATOM cls = register_main_window_class(hInstance);
        ::HWND hwnd = create_main_window(hInstance, cls);

        ::ShowWindow(hwnd, nCmdShow);
        ::UpdateWindow(hwnd);

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
        ::MessageBoxA(0, e.what(), "Sound Player", MB_OK);
    }
    return 0;
}
