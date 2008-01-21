// icon_view_window.cpp: the window for icon view

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "icon_view_window.hpp"
#include "icon_select_dialog.hpp"
#include "icon_view_window_impl.hpp"

#include <hamigaki/system/windows_error.hpp>

using hamigaki::system::windows_error;

namespace
{

::LRESULT CALLBACK window_proc(
    ::HWND hwnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        icon_view_window* pimpl =
            reinterpret_cast<icon_view_window*>(
                GetWindowLongPtr(hwnd, GWLP_USERDATA)
            );

        if (uMsg == WM_GETMINMAXINFO)
        {
            ::MINMAXINFO& info = *reinterpret_cast< ::MINMAXINFO*>(lParam);
            info.ptMaxSize.x = 32;
            info.ptMaxSize.y = 32;
            info.ptMinTrackSize.x = 32;
            info.ptMinTrackSize.y = 32;
            info.ptMaxTrackSize.x = 32;
            info.ptMaxTrackSize.y = 32;
            return 0;
        }
        else if (uMsg == WM_CREATE)
        {
            pimpl = new icon_view_window(hwnd);
            SetWindowLongPtr(
                hwnd, GWLP_USERDATA, reinterpret_cast< ::LONG_PTR>(pimpl));
        }
        else if (uMsg == WM_DESTROY)
        {
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            delete pimpl;
        }
        else if (pimpl)
        {
            if (uMsg == WM_PAINT)
                pimpl->render();
            else if (uMsg == WM_LBUTTONDBLCLK)
            {
                const rectangle<int>& r = pimpl->icon_rect();
                icon_info info;
                info.filename = pimpl->filename();
                info.x = r.x / 32;
                info.y = r.y / 32;

                if (select_icon(hwnd, info))
                {
                    rectangle<int> r;
                    r.x = info.x * 32;
                    r.y = info.y * 32;
                    r.lx = 32;
                    r.ly = 32;

                    pimpl->load(info.filename, r);
                }
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

::ATOM register_icon_view_window_class(::HINSTANCE hInstance)
{
    ::WNDCLASSEXA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = &window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(::LONG_PTR);
    wc.hInstance = hInstance;
    wc.hIcon = 0;
    wc.hCursor = ::LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = 0;
    wc.lpszMenuName = 0;
    wc.lpszClassName = "IconViewWindow";
    wc.hIconSm = 0;

    ::ATOM cls = ::RegisterClassExA(&wc);
    if (cls == 0)
        throw windows_error(::GetLastError(), "RegisterClassExA()");

    return cls;
}

void icon_window_load(
    ::HWND hwnd, const std::string& filename, const rectangle<int>& r)
{
    icon_view_window* pimpl =
        reinterpret_cast<icon_view_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        pimpl->load(filename, r);
}

std::string icon_window_filename(::HWND hwnd)
{
    icon_view_window* pimpl =
        reinterpret_cast<icon_view_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        return pimpl->filename();
    else
        return std::string();
}

rectangle<int> icon_window_icon_rect(::HWND hwnd)
{
    icon_view_window* pimpl =
        reinterpret_cast<icon_view_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        return pimpl->icon_rect();
    else
        return rectangle<int>();
}
