// direct3d_device9.hpp: Direct3D 9 device class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef DIRECT3D_DEVICE9_HPP
#define DIRECT3D_DEVICE9_HPP

#include "direct3d_texture9.hpp"
#include <boost/noncopyable.hpp>
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

    void clear_target(::D3DCOLOR color)
    {
        ::HRESULT res = pimpl_->Clear(0, 0, D3DCLEAR_TARGET, color, 1.0f, 0);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::Clear()");
    }

    void begin_scene()
    {
        ::HRESULT res = pimpl_->BeginScene();
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::BeginScene()");
    }

    void end_scene()
    {
        ::HRESULT res = pimpl_->EndScene();
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::EndScene()");
    }

    void present()
    {
        ::HRESULT res = pimpl_->Present(0, 0, 0, 0);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::Present()");
    }

    direct3d_texture9 create_texture(
        unsigned width, unsigned height, unsigned levels,
        unsigned long usage, ::D3DFORMAT format, ::D3DPOOL pool)
    {
        ::IDirect3DTexture9* tmp = 0;
        ::HRESULT res = pimpl_->CreateTexture(
            width, height, levels, usage, format, pool, &tmp, 0);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::CreateTexture()");
        return direct3d_texture9(tmp);
    }

    void set_texture(const direct3d_texture9& texture, unsigned long sampler)
    {
        texture.set_to_device(pimpl_.get(), sampler);
    }

    void clear_texture(unsigned long sampler)
    {
        ::HRESULT res = pimpl_->SetTexture(sampler, 0);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::SetTexture()");
    }

    void set_vertex_format(unsigned long fmt)
    {
        ::HRESULT res = pimpl_->SetFVF(fmt);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::SetFVF()");
    }

    void set_render_state(::D3DRENDERSTATETYPE type, unsigned long value)
    {
        ::HRESULT res = pimpl_->SetRenderState(type, value);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::SetRenderState()");
    }

    void set_texture_stage_state(
        unsigned long stage,
        ::D3DTEXTURESTAGESTATETYPE type, unsigned long value)
    {
        ::HRESULT res = pimpl_->SetTextureStageState(stage, type, value);
        if (FAILED(res))
        {
            throw directx9_error(
                res, "IDirect3DDevice9::SetTextureStageState()");
        }
    }

    void draw_primitive(::D3DPRIMITIVETYPE type,
        unsigned count, const void* data, unsigned stride)
    {
        ::HRESULT res = pimpl_->DrawPrimitiveUP(type, count, data, stride);
        if (FAILED(res))
            throw directx9_error(res, "IDirect3DDevice9::DrawPrimitiveUP()");
    }

private:
    boost::shared_ptr< ::IDirect3DDevice9> pimpl_;
};

class scoped_scene : boost::noncopyable
{
public:
    explicit scoped_scene(direct3d_device9& dev) : device_(dev)
    {
        device_.begin_scene();
    }

    ~scoped_scene()
    {
        try
        {
            device_.end_scene();
        }
        catch (...)
        {
        }
    }

private:
    direct3d_device9& device_;
};

#endif // DIRECT3D_DEVICE9_HPP
