// combo_box.hpp: combo box operations

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef COMBO_BOX_HPP
#define COMBO_BOX_HPP

#include <stdexcept>
#include <windows.h>

namespace combo_box {

inline ::UINT_PTR add_string(::HWND hwnd, const char* s)
{
    ::LRESULT res =
        ::SendMessage(hwnd, CB_ADDSTRING, 0, reinterpret_cast< ::LPARAM>(s));

    if (res == CB_ERR)
        throw std::runtime_error("cannot add the string to the combo box");
    else if (res == CB_ERRSPACE)
        throw std::runtime_error("out of memory for the combo box");

    return static_cast< ::UINT_PTR>(res);
}

inline void reset_content(::HWND hwnd)
{
    ::SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
}

inline void select_by_index(::HWND hwnd, ::UINT_PTR index)
{
    ::LRESULT res = ::SendMessage(hwnd, CB_SETCURSEL, index, 0);
    if (res == CB_ERR)
        throw std::out_of_range("bad index for the combo box");
}

inline ::UINT_PTR selected_index(::HWND hwnd)
{
    ::LRESULT res = ::SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    if (res == CB_ERR)
        throw std::runtime_error("no item is selected for the combo box");

    return static_cast< ::UINT_PTR>(res);
}

} // End namespace combo_box

#endif // COMBO_BOX_HPP
