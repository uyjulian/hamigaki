// map_edit_window.cpp: the window for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <hamigaki/system/windows_error.hpp>
#include "map_edit_window.hpp"
#include "map_edit_window_impl.hpp"

using hamigaki::system::windows_error;

namespace
{

::LRESULT CALLBACK window_proc(
    ::HWND hwnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        map_edit_window* pimpl =
            reinterpret_cast<map_edit_window*>(
                GetWindowLongPtr(hwnd, GWLP_USERDATA)
            );

        if (uMsg == WM_CREATE)
        {
            pimpl = new map_edit_window(hwnd);
            SetWindowLongPtr(
                hwnd, GWLP_USERDATA, reinterpret_cast< ::LONG_PTR>(pimpl));
        }
        else if (uMsg == WM_DESTROY)
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            delete pimpl;
        }
        else if (uMsg == WM_PAINT)
        {
            if (pimpl)
                pimpl->render();
        }
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwnd, e.what(), "Action Game Editor", MB_OK);
    }
    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

} // namespace

::ATOM register_map_edit_window_class(::HINSTANCE hInstance)
{
    ::WNDCLASSEXA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(::LONG_PTR);
    wc.hInstance = hInstance;
    wc.hIcon = 0;
    wc.hCursor = ::LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = "MapWindow";
    wc.hIconSm = 0;

    ::ATOM cls = ::RegisterClassExA(&wc);
    if (cls == 0)
        throw windows_error(::GetLastError(), "RegisterClassExA()");

    return cls;
}

::HWND create_map_edit_window(
    ::HWND parent, int left, ::HINSTANCE hInstance, ::ATOM cls)
{
    ::RECT cr;
    ::GetClientRect(parent, &cr);

    ::DWORD style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;
    ::DWORD ex_style = WS_EX_CLIENTEDGE;

    ::HWND hwnd = ::CreateWindowExA(
        ex_style, MAKEINTATOM(cls), "", style,
        left, 0, cr.right - cr.left - left, cr.bottom - cr.top,
        parent, 0, hInstance, 0
    );
    if (hwnd == 0)
        throw windows_error(::GetLastError(), "CreateWindowExA()");

    return hwnd;
}

void connect_d3d_device_map_edit_window(::HWND hwnd)
{
    map_edit_window* pimpl =
        reinterpret_cast<map_edit_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl)
        pimpl->connect_d3d_device();
}
