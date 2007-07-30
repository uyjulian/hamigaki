// direct_input.hpp: DirectInput classes

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/ for library home page.

#ifndef DIRECT_INPUT_HPP
#define DIRECT_INPUT_HPP

#include "directx9_error.hpp"
#include <hamigaki/coroutine/generator.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <dinput.h>

namespace direct_input
{

typedef ::DIDEVICEINSTANCEA device_info;

typedef hamigaki::coroutines::generator<
    device_info
> device_info_iterator;

class direct_input2 : private boost::noncopyable
{
public:
    explicit direct_input2(::HINSTANCE hInstance) : pimpl_(0)
    {
        ::IDirectInputA* base;
        ::HRESULT res =
            ::DirectInputCreateA(hInstance, DIRECTINPUT_VERSION, &base, 0);

        // IID_IDirectInput2A
        const ::GUID iid =
        {
            0x5944E662, 0xAA8A, 0x11CF,
            { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }
        };

        void* tmp = 0;
        res = base->QueryInterface(iid, &tmp);
        base->Release();
        if (FAILED(res))
            throw directx9_error(res, "IDirectInputA::QueryInterface()");

        pimpl_ = static_cast< ::IDirectInput2A*>(tmp);
    }

    ~direct_input2()
    {
        pimpl_->Release();
        pimpl_ = 0;
    }

    std::pair<device_info_iterator,device_info_iterator>
    device_info_range(unsigned long type, unsigned long flags)
    {
        return std::pair<device_info_iterator,device_info_iterator>(
            device_info_iterator(
                boost::bind(
                    &direct_input2::enum_devices, this, _1, type, flags
                )
            ),
            device_info_iterator()
        );
    }

private:
    ::IDirectInput2A* pimpl_;

    static int __stdcall enum_devices_callback(
        const device_info* lpddi, void* pvRef)
    {
        try
        {
            device_info_iterator::self& self =
                *reinterpret_cast<device_info_iterator::self*>(pvRef);

            self.yield(*lpddi);

            return TRUE;
        }
        catch (...)
        {
        }

        return FALSE;
    }

    device_info enum_devices(
        device_info_iterator::self& self,
        unsigned long type, unsigned long flags
    )
    {
        ::HRESULT res =
            pimpl_->EnumDevices(type, &enum_devices_callback, &self, flags);
        if (FAILED(res))
            throw directx9_error(res, "IDirectInput2A::EnumDevices()");

        self.exit();
        HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(device_info())
    }
};

} // namespace direct_input

#endif // DIRECT_INPUT_HPP
