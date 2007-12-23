// msg_utilities.hpp: some utilities for Window Messages

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef MSG_UTILITIES_HPP
#define MSG_UTILITIES_HPP

#include <string>
#include <windows.h>

inline ::LRESULT
send_msg(::HWND hwnd, ::UINT msg, ::WPARAM wp = 0, ::LPARAM lp = 0)
{
    return ::SendMessageA(hwnd, msg, wp, lp);
}

inline ::LRESULT
send_msg(::HWND hwnd, ::UINT msg, ::WPARAM wp, const std::string& lp)
{
    return ::SendMessageA(
        hwnd, msg, wp, reinterpret_cast< ::LONG_PTR>(lp.c_str()));
}

inline ::LRESULT
send_msg_with_ptr(::HWND hwnd, ::UINT msg, ::WPARAM wp, void* lp)
{
    return ::SendMessageA(hwnd, msg, wp, reinterpret_cast< ::LONG_PTR>(lp));
}

inline ::LRESULT
send_msg_with_ptr(::HWND hwnd, ::UINT msg, ::WPARAM wp, const void* lp)
{
    return ::SendMessageA(hwnd, msg, wp, reinterpret_cast< ::LONG_PTR>(lp));
}


inline ::LRESULT
send_command(::HWND hwnd, int id, int code, ::HWND lp)
{
    return send_msg_with_ptr(hwnd, WM_COMMAND, MAKEWPARAM(id, code), lp);
}

#endif // MSG_UTILITIES_HPP
