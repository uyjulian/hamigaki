// char_select_window.cpp: the window for character selection

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include "char_select_window.hpp"
#include "char_class_dialog.hpp"
#include "char_select_window_impl.hpp"

#include <hamigaki/system/windows_error.hpp>

using hamigaki::system::windows_error;

namespace
{

::LRESULT CALLBACK window_proc(
    ::HWND hwnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    try
    {
        char_select_window* pimpl =
            reinterpret_cast<char_select_window*>(
                GetWindowLongPtr(hwnd, GWLP_USERDATA)
            );

        if (uMsg == WM_CREATE)
        {
            pimpl = new char_select_window(hwnd);
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
            else if (uMsg == WM_LBUTTONDOWN)
            {
                int x = LOWORD(lParam) / 32;
                int y = HIWORD(lParam) / 32;
                pimpl->cursor_pos(x, y);
            }
            else if (uMsg == WM_LBUTTONDBLCLK)
            {
                if (game_character_class* p = pimpl->selected_char())
                {
                    game_character_class info(*p);
                    if (get_character_class_info(hwnd, info))
                    {
                        *p = info;
                        p->modified = true;
                        pimpl->modified(true);
                        ::InvalidateRect(hwnd, 0, FALSE);
                    }
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

::ATOM register_char_select_window_class(::HINSTANCE hInstance)
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
    wc.lpszClassName = "CharSelectWindow";
    wc.hIconSm = 0;

    ::ATOM cls = ::RegisterClassExA(&wc);
    if (cls == 0)
        throw windows_error(::GetLastError(), "RegisterClassExA()");

    return cls;
}

::HWND create_char_select_window(::HWND parent, int id, ::HINSTANCE hInstance)
{
    ::DWORD style = WS_CHILD | WS_VISIBLE | WS_VSCROLL;
    ::DWORD ex_style = WS_EX_CLIENTEDGE;

    ::RECT r = { 0, 0, 128, 256 };
    ::AdjustWindowRectEx(&r, style, FALSE, ex_style);

    r.right += ::GetSystemMetrics(SM_CXVSCROLL);

    ::HWND hwnd = ::CreateWindowExA(
        ex_style, "CharSelectWindow", "", style,
        0, 0, r.right - r.left, r.bottom - r.top,
        parent,
        reinterpret_cast< ::HMENU>(static_cast< ::LONG_PTR>(id)),
        hInstance, 0
    );
    if (hwnd == 0)
        throw windows_error(::GetLastError(), "CreateWindowExA()");

    return hwnd;
}

hamigaki::uuid get_selected_char(::HWND hwnd)
{
    char_select_window* pimpl =
        reinterpret_cast<char_select_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
    {
        if (game_character_class* p = pimpl->selected_char())
            return p->id;
        else
            return hamigaki::uuid();
    }
    else
        return hamigaki::uuid();
}

void setup_char_list(::HWND hwnd, std::set<game_character_class>* chars)
{
    char_select_window* pimpl =
        reinterpret_cast<char_select_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        pimpl->set_characters(chars);
}

bool char_select_window_modified(::HWND hwnd)
{
    char_select_window* pimpl =
        reinterpret_cast<char_select_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        return pimpl->modified();
    else
        return false;
}

void char_select_window_modified(::HWND hwnd, bool value)
{
    char_select_window* pimpl =
        reinterpret_cast<char_select_window*>(
            GetWindowLongPtr(hwnd, GWLP_USERDATA)
        );

    if (pimpl != 0)
        pimpl->modified(value);
}
