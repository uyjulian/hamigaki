// list_box.hpp: list box operations

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef LIST_BOX_HPP
#define LIST_BOX_HPP

#include <stdexcept>
#include <string>
#include <windows.h>

namespace list_box {

void enable_horizontal_scroll_bar(::HWND hwnd);

inline ::UINT_PTR add_string(::HWND hwnd, const char* s)
{
    ::LRESULT res =
        ::SendMessage(hwnd, LB_ADDSTRING, 0, reinterpret_cast< ::LPARAM>(s));

    if (res == LB_ERR)
        throw std::runtime_error("cannot add the string to the list box");
    else if (res == LB_ERRSPACE)
        throw std::runtime_error("out of memory for the list box");

    return static_cast< ::UINT_PTR>(res);
}

inline ::UINT_PTR add_string(::HWND hwnd, const std::string& s)
{
    return add_string(hwnd, s.c_str());
}

inline void reset_content(::HWND hwnd)
{
    ::SendMessage(hwnd, LB_RESETCONTENT, 0, 0);
}

inline void anchor_index(::HWND hwnd, ::UINT_PTR index)
{
    ::LRESULT res = ::SendMessage(hwnd, LB_SETANCHORINDEX, index, 0);

    if (res == LB_ERR)
        throw std::runtime_error("cannot set the anchor for the list box");
}

} // End namespace list_box

#endif // LIST_BOX_HPP
