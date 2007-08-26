// list_box.cpp: the list box with the horizontal scroll bar

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#include <cstring>
#include <stdexcept>
#include "list_box.hpp"

namespace
{

const char old_wndproc_name[] = "HamigakiOldWndProc";

class memory_dc
{
public:
    explicit memory_dc(::HWND hwnd) : hwnd_(hwnd), handle_(::GetDC(hwnd))
    {
        if (handle_ == 0)
            throw std::runtime_error("GetDC() failed");
    }

    ~memory_dc()
    {
        ::ReleaseDC(hwnd_, handle_);
    }

    ::SIZE text_extent(const char* s, std::size_t n)
    {
        ::SIZE buf;
        ::GetTextExtentPoint32A(handle_, s, static_cast<int>(n), &buf);
        return buf;
    }

    ::HFONT select_font(::HFONT font)
    {
        return
            reinterpret_cast< ::HFONT>(
                ::SelectObject(handle_, reinterpret_cast< ::HGDIOBJ>(font))
            );
    }

private:
    ::HWND hwnd_;
    ::HDC handle_;
};

class scoped_select_font
{
public:
    scoped_select_font(memory_dc& dc, ::HFONT font)
        : dc_(dc), font_(dc_.select_font(font))
    {
    }

    ~scoped_select_font()
    {
        dc_.select_font(font_);
    }

private:
    memory_dc dc_;
    ::HFONT font_;
};

::HFONT get_font(::HWND hwnd)
{
    return reinterpret_cast< ::HFONT>(::SendMessage(hwnd, WM_GETFONT, 0, 0));
}

::LONG get_text_width(::HWND hwnd, const char* s)
{
    memory_dc dc(hwnd);
    scoped_select_font using_font(dc, get_font(hwnd));
    return dc.text_extent(s, std::strlen(s)).cx;
}

::LRESULT CALLBACK window_proc(
    ::HWND hwnd, ::UINT uMsg, ::WPARAM wParam, ::LPARAM lParam)
{
    ::WNDPROC old =
        reinterpret_cast< ::WNDPROC>(
            reinterpret_cast< ::LONG_PTR>(::GetPropA(hwnd, old_wndproc_name))
        );

    try
    {
        if (uMsg == WM_DESTROY)
        {
            ::RemovePropA(hwnd, old_wndproc_name);

            SetWindowLongPtr(
                hwnd, GWLP_WNDPROC, reinterpret_cast< ::LONG_PTR>(old));
        }
        else if (uMsg == LB_ADDSTRING)
        {
            const char* s = reinterpret_cast<const char*>(lParam);
            ::LONG width = get_text_width(hwnd, s) + 4;
            ::LONG old = ::SendMessage(hwnd, LB_GETHORIZONTALEXTENT, 0, 0);
            if (width > old)
                ::SendMessage(hwnd, LB_SETHORIZONTALEXTENT, width, 0);
        }
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(hwnd, e.what(), "bjam for Windows", MB_OK);
    }

    if (old)
        return ::CallWindowProc(old, hwnd, uMsg, wParam, lParam);
    else
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

} // namespace

namespace list_box {

void enable_horizontal_scroll_bar(::HWND hwnd)
{
    ::LONG_PTR old = SetWindowLongPtr(
        hwnd, GWLP_WNDPROC, reinterpret_cast< ::LONG_PTR>(&window_proc));

    ::SetPropA(hwnd, old_wndproc_name, reinterpret_cast< ::HANDLE>(old));
}

} // End namespace list_box
