// rebar.hpp: rebar operations

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef REBAR_HPP
#define REBAR_HPP

#include <stdexcept>
#include <windows.h>
#include <commctrl.h>

namespace rebar {

inline void bar_info(::HWND hwnd, const ::REBARINFO& info)
{
    ::LRESULT res = ::SendMessage(
        hwnd, RB_SETBARINFO, 0, reinterpret_cast< ::LPARAM>(&info));
    if (res == 0)
        throw std::runtime_error("cannot set the rebar info");
}

inline void insert_band(
    ::HWND hwnd, ::UINT_PTR index, const ::REBARBANDINFOA& info)
{
    ::LRESULT res = ::SendMessage(
        hwnd, RB_INSERTBAND, index, reinterpret_cast< ::LPARAM>(&info));
    if (res == 0)
        throw std::runtime_error("cannot insert the band to the rebar");
}

inline void add_band(::HWND hwnd, const ::REBARBANDINFOA& info)
{
    insert_band(
        hwnd, static_cast< ::UINT_PTR>(static_cast< ::INT_PTR>(-1)), info);
}

} // End namespace rebar

#endif // REBAR_HPP
