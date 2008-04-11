// winnls.hpp: Windows NLS functions

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#ifndef HAMIGAKI_CHARSET_DETAIL_WINNLS_HPP
#define HAMIGAKI_CHARSET_DETAIL_WINNLS_HPP

extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(
    unsigned, unsigned long, const wchar_t*, int,
    char*, int, const char*, int*);

extern "C" __declspec(dllimport) int __stdcall MultiByteToWideChar(
    unsigned, unsigned long, const char*, int, wchar_t*, int);

#endif // HAMIGAKI_CHARSET_DETAIL_WINNLS_HPP
