// sound_player.cpp: a GUI example for background_player

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#include <boost/scoped_array.hpp>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "main_window.hpp"
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
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
        "Sound Files\0*.wav;*.ogg;*.aiff;*.aif\0"
        "WAVE Files (*.wav)\0*.wav\0"
        "Ogg Files (*.ogg)\0*.ogg\0"
        "AIFF Files (*.aiff;*.aif)\0*.aiff;*.aif\0"
        ;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.lpstrFile = buf;
    ofn.nMaxFile = sizeof(buf);

    if (::GetOpenFileNameA(&ofn) == FALSE)
        return false;

    filename = buf;
    return true;
}

std::string get_drop_filename(::HDROP drop)
{
    ::UINT size = ::DragQueryFileA(drop, 0, 0, 0);
    boost::scoped_array<char> buf(new char[size+1]);
    ::DragQueryFileA(drop, 0, buf.get(), size+1);
    buf[size] = '\0';

    return buf.get();
}

void show_properties(::HWND hwnd, const audio_info& info)
{
    std::ostringstream os;
    os
        << "container:\t" << info.container << '\n'
        << "encoding:\t" << info.encoding << '\n'
        << "length:\t\t" << std::setfill('0')
            << std::setw(2) << (info.length/3600) << ':'
            << std::setw(2) << (info.length/60%60) << ':'
            << std::setw(2) << (info.length%60) << std::setfill(' ') << '\n'
        << "bit rate:\t\t" << (info.bit_rate/1000) << "kbps\n"
        << "quantization bit:\t" << info.bits << " bit\n"
        << "sampling rate:\t" << (info.sampling_rate/1000) << "kHz\n"
        << "channel:\t\t" << info.channels
        ;

    if (info.channels == 1)
        os << " (Mono)";
    else if (info.channels == 2)
        os << " (Stereo)";

    ::MessageBoxA(hwnd, os.str().c_str(), "Properties", MB_OK);
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
            else if (code == ID_FILE_PROP)
                ::show_properties(hwnd, pimpl->info());
            else if (code == ID_FILE_EXIT)
                ::DestroyWindow(hwnd);
        }
        else if (uMsg == WM_DROPFILES)
        {
            ::HDROP drop = reinterpret_cast< ::HDROP>(wParam);
            std::string filename = get_drop_filename(drop);
            ::DragFinish(drop);

            pimpl->open(filename);
            pimpl->play();
        }
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwnd, e.what(), "Sound Player", MB_OK);
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

    ::DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    ::DWORD ex_style = WS_EX_ACCEPTFILES;

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
