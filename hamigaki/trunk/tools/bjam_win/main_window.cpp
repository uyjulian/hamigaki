//  main_window_class.cpp: main window for bjam_win

//  Copyright Takeshi Mouri 2007.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/ for library home page.

#include "main_window_impl.hpp"
#include "main_window.hpp"
#include "menus.h"

namespace
{

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
        "Jamfile.v2\0Jamfile.v2\0"
        "All Files (*.*)\0*.*\0"
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
            if (code == ID_FILE_LOAD)
            {
                std::string filename;
                if (get_open_file_name(hwnd, filename))
                    pimpl->open(filename);
            }
            else if (code == ID_FILE_EXIT)
                ::DestroyWindow(hwnd);
            else if (code == ID_BUILD_RUN)
                pimpl->run();
            else if (code == ID_BUILD_STOP)
                pimpl->stop();
        }
        else if (uMsg == WM_SIZE)
        {
            if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED))
            {
                if (pimpl)
                    pimpl->update_size();
            }
        }
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwnd, e.what(), "bjam for Windows", MB_OK);
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
    ::DWORD style = WS_OVERLAPPEDWINDOW;
    ::DWORD ex_style = 0;

    ::HWND hwnd = ::CreateWindowEx(
        ex_style, MAKEINTATOM(cls), "bjam for Windows", style,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        0, 0, hInstance, 0
    );
    if (hwnd == 0)
        throw std::runtime_error("CreateWindowEx() failed");

    return hwnd;
}
