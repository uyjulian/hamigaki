// direct3d_device9.hpp: Direct3D 9 device class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/system for library home page.

#ifndef DIRECT3D_DEVICE9_HPP
#define DIRECT3D_DEVICE9_HPP

#include <boost/shared_ptr.hpp>
#include <hamigaki/detail/windows/com_release.hpp>
#include <windows.h>
#include <d3d9.h>

class direct3d_device9
{
public:
    direct3d_device9()
    {
    }

    direct3d_device9(::IDirect3DDevice9* p)
        : pimpl_(p, hamigaki::detail::windows::com_release())
    {
    }

private:
    boost::shared_ptr< ::IDirect3DDevice9> pimpl_;
};

#endif // DIRECT3D_DEVICE9_HPP
