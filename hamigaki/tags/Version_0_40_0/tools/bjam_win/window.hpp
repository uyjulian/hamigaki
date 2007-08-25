// window.hpp: basic window operations

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <boost/scoped_array.hpp>
#include <stdexcept>
#include <string>
#include <windows.h>

namespace window {

inline ::HWND create_child(
    ::DWORD ex_style, const char* class_name, const char* text,
    ::DWORD style, int x, int y, int width, int height,
    ::HWND parent, ::UINT_PTR id, ::HINSTANCE hInstance, void* param=0)
{
    ::HWND hwnd = ::CreateWindowExA(
        ex_style, class_name, text,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | style,
        x, y, width, height, parent,
        reinterpret_cast< ::HMENU>(id),
        hInstance, param
    );

    if (hwnd == 0)
        throw std::runtime_error("cannot create a window");

    return hwnd;
}

inline ::UINT_PTR text_length(::HWND hwnd)
{
    ::LRESULT res = ::SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
    return static_cast< ::UINT_PTR>(res);
}

inline ::UINT_PTR get_text(::HWND hwnd, char* buf, ::UINT_PTR size)
{
    ::LRESULT res = ::SendMessage(
        hwnd, WM_GETTEXT, size, reinterpret_cast< ::LPARAM>(buf));
    return static_cast< ::UINT_PTR>(res);
}

inline std::string get_text(::HWND hwnd)
{
    ::UINT_PTR size = text_length(hwnd);

    boost::scoped_array<char> buf(new char[size+1]);
    size = get_text(hwnd, buf.get(), size + 1);

    return std::string(buf.get(), size);
}

inline void set_text(::HWND hwnd, const char* s)
{
    if (::SetWindowTextA(hwnd, s) == FALSE)
        throw std::runtime_error("cannot set the string to the window");
}

inline void set_text(::HWND hwnd, const std::string& s)
{
    set_text(hwnd, s.c_str());
}

inline ::RECT bounding_rect(::HWND hwnd)
{
    ::RECT rect;
    if (::GetWindowRect(hwnd, &rect) == FALSE)
    {
        throw std::runtime_error(
            "cannot retrieves the bounding rectangle of the window");
    }
    return rect;
}

inline ::SIZE size(::HWND hwnd)
{
    const ::RECT& rect = bounding_rect(hwnd);

    ::SIZE sz;
    sz.cx = rect.right - rect.left;
    sz.cy = rect.bottom - rect.top;

    return sz;
}

inline ::LONG width(::HWND hwnd)
{
    return size(hwnd).cx;
}

inline ::LONG height(::HWND hwnd)
{
    return size(hwnd).cy;
}

inline ::SIZE client_area_size(::HWND hwnd)
{
    ::RECT rect;
    if (::GetClientRect(hwnd, &rect) == FALSE)
    {
        throw std::runtime_error(
            "cannot retrieves the coordinates of the window's client area");
    }

    ::SIZE sz;
    sz.cx = rect.right;
    sz.cy = rect.bottom;

    return sz;
}

inline ::LONG client_area_width(::HWND hwnd)
{
    return client_area_size(hwnd).cx;
}

inline ::LONG client_area_height(::HWND hwnd)
{
    return client_area_size(hwnd).cy;
}

} // End namespace window

#endif // WINDOW_HPP
