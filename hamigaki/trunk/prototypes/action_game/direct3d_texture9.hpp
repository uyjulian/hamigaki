// direct3d_texture9.hpp: Direct3D 9 texture class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/system for library home page.

#ifndef DIRECT3D_TEXTURE9_HPP
#define DIRECT3D_TEXTURE9_HPP

#include "directx9_error.hpp"
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <hamigaki/detail/windows/com_release.hpp>
#include <windows.h>
#include <d3d9.h>

class direct3d_texture9
{
public:
    class scoped_lock : boost::noncopyable
    {
    public:
        scoped_lock(
            direct3d_texture9& tex, unsigned level, unsigned long flags=0
        )
            : texture_(tex), level_(level)
            , rect_(texture_.lock(level_, flags))
        {
        }

        ~scoped_lock()
        {
            try
            {
                texture_.unlock(level_);
            }
            catch (...)
            {
            }
        }

        int pitch() const
        {
            return rect_.Pitch;
        }

        void* address()
        {
            return rect_.pBits;
        }

        const void* address() const
        {
            return rect_.pBits;
        }

    private:
        direct3d_texture9& texture_;
        unsigned level_;
        ::D3DLOCKED_RECT rect_;
    };

    direct3d_texture9()
    {
    }

    direct3d_texture9(::IDirect3DTexture9* p)
        : pimpl_(p, hamigaki::detail::windows::com_release())
    {
    }

    ::D3DLOCKED_RECT lock(unsigned level, unsigned long flags=0)
    {
        ::D3DLOCKED_RECT rect;
        ::HRESULT res = pimpl_->LockRect(level, &rect, 0, flags);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DTexture9::LockRect()");
        return rect;
    }

    void unlock(unsigned level)
    {
        ::HRESULT res = pimpl_->UnlockRect(level);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DTexture9::UnlockRect()");
    }

    void set_to_device(::IDirect3DDevice9* device, unsigned long sampler) const
    {
        ::HRESULT res = device->SetTexture(sampler, pimpl_.get());
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::SetTexture()");
    }

    ::D3DSURFACE_DESC description(unsigned level)
    {
        ::D3DSURFACE_DESC tmp;
        ::HRESULT res = pimpl_->GetLevelDesc(level, &tmp);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DTexture9::GetLevelDesc()");
        return tmp;
    }

private:
    boost::shared_ptr< ::IDirect3DTexture9> pimpl_;
};

#endif // DIRECT3D_TEXTURE9_HPP
