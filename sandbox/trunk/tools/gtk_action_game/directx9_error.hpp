// directx9_error.hpp: DirectX9 error class

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef DIRECTX9_ERROR_HPP
#define DIRECTX9_ERROR_HPP

#include <hamigaki/system/system_error.hpp>
#include <windows.h>
#include <dxerr9.h>

struct directx9_error_traits
{
    typedef long value_type;

    static std::string message(long code)
    {
        return ::DXGetErrorDescription9A(code);
    }
};

typedef hamigaki::system::system_error<directx9_error_traits> directx9_error;

#endif // DIRECTX9_ERROR_HPP
