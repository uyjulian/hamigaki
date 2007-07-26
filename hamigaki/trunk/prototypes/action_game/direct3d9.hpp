// direct3d9.hpp: Direct3D 9 class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/system for library home page.

#ifndef DIRECT3D9_HPP
#define DIRECT3D9_HPP

#include "direct3d_device9.hpp"
#include "directx9_error.hpp"
#include <boost/noncopyable.hpp>
#include <stdexcept>

class direct3d9 : private boost::noncopyable
{
public:
    direct3d9() : pimpl_(::Direct3DCreate9(D3D_SDK_VERSION))
    {
        if (!pimpl_)
            throw std::runtime_error("Direct3DCreate9() failed");
    }

    ~direct3d9()
    {
        pimpl_->Release();
        pimpl_ = 0;
    }

    direct3d_device9 create_device(
        unsigned adapter, ::D3DDEVTYPE type, ::HWND hwnd,
        unsigned long flags, ::D3DPRESENT_PARAMETERS& params)
    {
        ::IDirect3DDevice9* tmp = 0;

        ::HRESULT res =
            pimpl_->CreateDevice(adapter, type, hwnd, flags, &params, &tmp);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3D9::CreateDevice()");

        return direct3d_device9(tmp);
    }

private:
    ::IDirect3D9* pimpl_;
};

#endif // DIRECT3D9_HPP
