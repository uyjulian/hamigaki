// map_edit_window.cpp: the window for stage map

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <hamigaki/system/windows_error.hpp>
#include "map_edit_window.hpp"
#include "map_edit_window_impl.hpp"
#include <boost/optional.hpp>
#include <algorithm>

using hamigaki::system::windows_error;

namespace
{

boost::optional<int> next_scroll_pos(::HWND hwnd, int bar, ::WORD cmd)
{
    if (cmd == SB_THUMBTRACK)
    {
        ::SCROLLINFO info = {};
        info.cbSize = sizeof(info);
        info.fMask = SIF_TRACKPOS;
        ::GetScrollInfo(hwnd, bar, &info);
        return info.nTrackPos;
    }

    ::SCROLLINFO info = {};
    info.cbSize = sizeof(info);
    info.fMask = SIF_POS|SIF_RANGE;
    ::GetScrollInfo(hwnd, bar, &info);

    switch (cmd)
    {
        case SB_LINELEFT:
            return (std::max)(info.nPos - 1, 0);
        case SB_LINERIGHT:
            return (std::min)(info.nPos + 1, info.nMax);
        case SB_PAGELEFT:
            return (std::max)(info.nPos - 10, 0);
        case SB_PAGERIGHT:
            return (std::min)(info.nPos + 10, info.nMax);
    }

    return boost::optional<int>();
}

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
        else if (pimpl)
        {
            if (uMsg == WM_PAINT)
                pimpl->render();
            else if (uMsg == WM_SIZE)
            {
                if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED))
                {
                    pimpl->reset_d3d();
                    pimpl->update_scroll_box();
                }
                return 0;
            }
            else if (uMsg == WM_HSCROLL)
            {
                ::WORD cmd = LOWORD(wParam);
                boost::optional<int> pos = next_scroll_pos(hwnd, SB_HORZ, cmd);
                if (pos)
                    pimpl->horz_scroll_pos(*pos);
                return 0;
            }
            else if (uMsg == WM_VSCROLL)
            {
                ::WORD cmd = LOWORD(wParam);
                boost::optional<int> pos = next_scroll_pos(hwnd, SB_VERT, cmd);
                if (pos)
                    pimpl->vert_scroll_pos(*pos);
                return 0;
            }
            else if (uMsg == WM_MOUSEMOVE)
            {
                ::RECT cr;
                ::GetClientRect(hwnd, &cr);

                int x = LOWORD(lParam) / 32;
                int y = (cr.bottom - HIWORD(lParam)) / 32;
                pimpl->cursor_pos(x, y);

                if (pimpl->mouse_captured())
                    pimpl->put_char();

                return 0;
            }
            else if (uMsg == WM_LBUTTONDOWN)
            {
                ::SetCapture(hwnd);
                pimpl->mouse_captured(true);
                pimpl->put_char();
                return 0;
            }
            else if (uMsg == WM_LBUTTONUP)
            {
                ::ReleaseCapture();
                return 0;
            }
            else if (uMsg == WM_CAPTURECHANGED)
            {
                pimpl->mouse_captured(false);
                return 0;
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
    ::HWND parent, int id, int left, ::HINSTANCE hInstance)
{
    ::RECT cr;
    ::GetClientRect(parent, &cr);

    ::DWORD style = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;
    ::DWORD ex_style = WS_EX_CLIENTEDGE;

    ::HWND hwnd = ::CreateWindowExA(
        ex_style, "MapWindow", "", style,
        left, 0, cr.right - left, cr.bottom,
        parent,
        reinterpret_cast< ::HMENU>(static_cast< ::LONG_PTR>(id)),
        hInstance, 0
    );
    if (hwnd == 0)
        throw windows_error(::GetLastError(), "CreateWindowExA()");

    return hwnd;
}

void map_edit_window_set(::HWND hwnd, stage_map* map)
{
    map_edit_window* pimpl =
        reinterpret_cast<map_edit_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        pimpl->set_stage(map);
}

void map_edit_window_set_char_list(
    ::HWND hwnd, std::set<game_character_class>* chars)
{
    map_edit_window* pimpl =
        reinterpret_cast<map_edit_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        pimpl->set_characters(chars);
}

void map_edit_window_select_char(::HWND hwnd, const hamigaki::uuid& c)
{
    map_edit_window* pimpl =
        reinterpret_cast<map_edit_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        pimpl->select_char(c);
}

bool map_edit_window_modified(::HWND hwnd)
{
    map_edit_window* pimpl =
        reinterpret_cast<map_edit_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        return pimpl->modified();
    else
        return false;
}
