// direct_input.cpp: DirectInput devices

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/input for library home page.

#define HAMIGAKI_INPUT_SOURCE
#include <hamigaki/input/direct_input.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>

#include <hamigaki/detail/windows/com_release.hpp>
#include <dinput.h>
#include <dxerr9.h>
#include <windows.h>

using namespace hamigaki::detail::windows;

namespace hamigaki { namespace input {

namespace direct_input
{

extern const ::DIDATAFORMAT joystick_data_format;

const unsigned long all_devices = DIEDFL_ALLDEVICES;
const unsigned long attached_only = DIEDFL_ATTACHEDONLY;
const unsigned long force_feedback_only = DIEDFL_FORCEFEEDBACK;

const unsigned long exclusive_level = DISCL_EXCLUSIVE;
const unsigned long nonexclusive_level = DISCL_NONEXCLUSIVE;
const unsigned long foreground_level = DISCL_FOREGROUND;
const unsigned long background_level = DISCL_BACKGROUND;

} // namespace direct_input

std::string direct_input_error_traits::message(long code)
{
    return ::DXGetErrorDescription9A(code);
}

class direct_input_manager::impl : private boost::noncopyable
{
public:
    typedef direct_input::device_info_iterator device_info_iterator;

    explicit impl(::HINSTANCE hInstance) : pimpl_(0)
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
            throw direct_input_error(res, "IDirectInputA::QueryInterface()");

        pimpl_ = static_cast< ::IDirectInput2A*>(tmp);
    }

    ~impl()
    {
        pimpl_->Release();
        pimpl_ = 0;
    }

    std::pair<device_info_iterator,device_info_iterator>
    devices(
        direct_input::device_type::values type, unsigned long flags)
    {
        return std::pair<device_info_iterator,device_info_iterator>(
            device_info_iterator(
                boost::bind(
                    &impl::enum_devices, this, _1, type, flags
                )
            ),
            device_info_iterator()
        );
    }

    ::IDirectInputDevice2A* create_joystick_device(const uuid& driver_guid)
    {
        ::GUID guid;
        driver_guid.copy(guid);

        ::IDirectInputDeviceA* base = 0;
        ::HRESULT res = pimpl_->CreateDevice(guid, &base, 0);
        if (FAILED(res))
            throw direct_input_error(res, "IDirectInput2A::CreateDevice()");

        // IID_IDirectInputDevice2A
        const ::GUID iid =
        {
            0x5944E682, 0xC92E, 0x11CF,
            { 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 }
        };

        void* tmp = 0;
        res = base->QueryInterface(iid, &tmp);
        base->Release();
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDeviceA::QueryInterface()");
        }

        return static_cast< ::IDirectInputDevice2A*>(tmp);
    }

private:
    ::IDirectInput2A* pimpl_;

    static int __stdcall enum_devices_callback(
        const ::DIDEVICEINSTANCEA* lpddi, void* pvRef)
    {
        try
        {
            device_info_iterator::self& self =
                *reinterpret_cast<device_info_iterator::self*>(pvRef);

            direct_input::device_info info;
            info.instance_guid = uuid(lpddi->guidInstance);
            info.product_guid = uuid(lpddi->guidProduct);
            info.type = lpddi->dwDevType;
            info.instance_name = lpddi->tszInstanceName;
            info.product_name = lpddi->tszProductName;
            info.ff_driver_guid = lpddi->guidFFDriver;
            info.usage_page = lpddi->wUsagePage;
            info.usage = lpddi->wUsage;

            self.yield(info);

            return TRUE;
        }
        catch (...)
        {
        }

        return FALSE;
    }

    direct_input::device_info enum_devices(
        device_info_iterator::self& self,
        unsigned long type, unsigned long flags
    )
    {
        ::HRESULT res =
            pimpl_->EnumDevices(
                type, &impl::enum_devices_callback, &self, flags);
        if (FAILED(res))
            throw direct_input_error(res, "IDirectInput2A::EnumDevices()");

        self.exit();
        HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(direct_input::device_info())
    }
};

direct_input_manager::direct_input_manager(void* instance)
    : pimpl_(new impl(static_cast< ::HINSTANCE>(instance)))
{
}

direct_input_manager::~direct_input_manager()
{
}

std::pair<direct_input::device_info_iterator,direct_input::device_info_iterator>
direct_input_manager::devices(
    direct_input::device_type::values type, unsigned long flags)
{
    return pimpl_->devices(type, flags);
}

std::pair<direct_input::device_info_iterator,direct_input::device_info_iterator>
direct_input_manager::devices(direct_input::device_type::values type)
{
    return pimpl_->devices(type, DIEDFL_ALLDEVICES);
}

direct_input_joystick
direct_input_manager::create_joystick_device(const uuid& driver_guid)
{
    return direct_input_joystick(pimpl_->create_joystick_device(driver_guid));
}


class direct_input_joystick::impl
{
public:
    explicit impl(const boost::shared_ptr< ::IDirectInputDevice2A>& ptr)
        : pimpl_(ptr)
    {
        ::HRESULT res =
            pimpl_->SetDataFormat(&direct_input::joystick_data_format);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::SetDataFormat()");
        }
    }

    ~impl()
    {
        try
        {
            unacquire();
        }
        catch (...)
        {
        }
    }

    void set_cooperative_level(::HWND hwnd, unsigned long level)
    {
        ::HRESULT res = pimpl_->SetCooperativeLevel(hwnd, level);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::SetCooperativeLevel()");
        }
    }

    std::pair<object_info_iterator,object_info_iterator>
    objects(unsigned long flags)
    {
        return std::pair<object_info_iterator,object_info_iterator>(
            object_info_iterator(
                boost::bind(&impl::enum_objects, this, _1, flags)
            ),
            object_info_iterator()
        );
    }

    std::pair<long,long> get_range(unsigned long type)
    {
        ::DIPROPRANGE data;
        std::memset(&data, 0, sizeof(data));
        data.diph.dwSize = sizeof(data);
        data.diph.dwHeaderSize = sizeof(::DIPROPHEADER);
        data.diph.dwObj = type;
        data.diph.dwHow = DIPH_BYID;

        ::HRESULT res = pimpl_->GetProperty(DIPROP_RANGE, &data.diph);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::GetProperty()");
        }

        return std::make_pair(data.lMin, data.lMax);
    }

    void set_range(unsigned long type, long min_val, long max_val)
    {
        ::DIPROPRANGE data;
        std::memset(&data, 0, sizeof(data));
        data.diph.dwSize = sizeof(data);
        data.diph.dwHeaderSize = sizeof(::DIPROPHEADER);
        data.diph.dwObj = type;
        data.diph.dwHow = DIPH_BYID;
        data.lMin = min_val;
        data.lMax = max_val;

        ::HRESULT res = pimpl_->SetProperty(DIPROP_RANGE, &data.diph);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::SetProperty()");
        }
    }

    unsigned long get_dword_property(const ::GUID& guid, unsigned long type)
    {
        ::DIPROPDWORD data;
        std::memset(&data, 0, sizeof(data));
        data.diph.dwSize = sizeof(data);
        data.diph.dwHeaderSize = sizeof(::DIPROPHEADER);
        data.diph.dwObj = type;
        data.diph.dwHow = DIPH_BYID;

        ::HRESULT res = pimpl_->GetProperty(guid, &data.diph);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::GetProperty()");
        }

        return data.dwData;
    }

    void set_dword_property(
        const ::GUID& guid, unsigned long type, unsigned long val)
    {
        ::DIPROPDWORD data;
        std::memset(&data, 0, sizeof(data));
        data.diph.dwSize = sizeof(data);
        data.diph.dwHeaderSize = sizeof(::DIPROPHEADER);
        data.diph.dwObj = type;
        data.diph.dwHow = DIPH_BYID;
        data.dwData = val;

        ::HRESULT res = pimpl_->SetProperty(guid, &data.diph);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::SetProperty()");
        }
    }

    void acquire()
    {
        ::HRESULT res = pimpl_->Acquire();
        if (FAILED(res))
            throw direct_input_error(res, "IDirectInputDevice2A::Acquire()");
    }

    void unacquire()
    {
        ::HRESULT res = pimpl_->Unacquire();
        if (FAILED(res))
            throw direct_input_error(res, "IDirectInputDevice2A::Unacquire()");
    }

    void get_state(direct_input::joystick_state& state)
    {
        ::HRESULT res = pimpl_->Poll();
        if ((res == DIERR_INPUTLOST) || (res == DIERR_NOTACQUIRED))
        {
            acquire();
            res = pimpl_->Poll();
        }
        if (FAILED(res))
            throw direct_input_error(res, "IDirectInputDevice2A::Poll()");

        ::DIJOYSTATE2 tmp;
        res = pimpl_->GetDeviceState(sizeof(tmp), &tmp);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::GetDeviceState()");
        }

        state.position.x = tmp.lX;
        state.position.y = tmp.lY;
        state.position.z = tmp.lZ;
        state.rotation.x = tmp.lRx;
        state.rotation.y = tmp.lRy;
        state.rotation.z = tmp.lRz;
        state.sliders[0] = tmp.rglSlider[0];
        state.sliders[1] = tmp.rglSlider[1];
        state.pov_directions[0] = tmp.rgdwPOV[0];
        state.pov_directions[1] = tmp.rgdwPOV[1];
        state.pov_directions[2] = tmp.rgdwPOV[2];
        state.pov_directions[3] = tmp.rgdwPOV[3];
        std::memcpy(state.buttons, tmp.rgbButtons, 128);
        state.velocity.x = tmp.lVX;
        state.velocity.y = tmp.lVY;
        state.velocity.z = tmp.lVZ;
        state.angular_velocity.x = tmp.lVRx;
        state.angular_velocity.y = tmp.lVRy;
        state.angular_velocity.z = tmp.lVRz;
        state.slider_velocities[0] = tmp.rglVSlider[0];
        state.slider_velocities[1] = tmp.rglVSlider[1];
        state.acceleration.x = tmp.lAX;
        state.acceleration.y = tmp.lAY;
        state.acceleration.z = tmp.lAZ;
        state.angular_acceleration.x = tmp.lARx;
        state.angular_acceleration.y = tmp.lARy;
        state.angular_acceleration.z = tmp.lARz;
        state.slider_accelerations[0] = tmp.rglASlider[0];
        state.slider_accelerations[1] = tmp.rglASlider[1];
        state.force.x = tmp.lFX;
        state.force.y = tmp.lFY;
        state.force.z = tmp.lFZ;
        state.torque.x = tmp.lFRx;
        state.torque.y = tmp.lFRy;
        state.torque.z = tmp.lFRz;
        state.slider_forces[0] = tmp.rglFSlider[0];
        state.slider_forces[1] = tmp.rglFSlider[1];
    }

private:
    boost::shared_ptr< ::IDirectInputDevice2A> pimpl_;

    static int __stdcall enum_objects_callback(
        const ::DIDEVICEOBJECTINSTANCEA* lpddoi, void* pvRef)
    {
        try
        {
            object_info_iterator::self& self =
                *reinterpret_cast<object_info_iterator::self*>(pvRef);

            direct_input::object_info info;

            info.type_guid = uuid(lpddoi->guidType);
            info.offset = lpddoi->dwOfs;
            info.type = lpddoi->dwType;
            info.flags = lpddoi->dwFlags;
            info.name = lpddoi->tszName;
            info.max_force = lpddoi->dwFFMaxForce;
            info.force_resolution = lpddoi->dwFFForceResolution;
            info.collection_number = lpddoi->wCollectionNumber;
            info.designator_index = lpddoi->wDesignatorIndex;
            info.usage_page = lpddoi->wUsagePage;
            info.usage = lpddoi->wUsage;
            info.dimension = lpddoi->dwDimension;
            info.exponent = lpddoi->wExponent;

            self.yield(info);

            return TRUE;
        }
        catch (...)
        {
        }

        return FALSE;
    }

    direct_input::object_info enum_objects(
        object_info_iterator::self& self, unsigned long flags
    )
    {
        ::HRESULT res =
            pimpl_->EnumObjects(&impl::enum_objects_callback, &self, flags);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::EnumObjects()");
        }

        self.exit();
        HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(direct_input::object_info())
    }
};

direct_input_joystick::direct_input_joystick(::IDirectInputDevice2A* p)
{
    boost::shared_ptr< ::IDirectInputDevice2A> tmp(p, com_release());

    pimpl_.reset(new impl(tmp));
}

direct_input_joystick::~direct_input_joystick()
{
}

void
direct_input_joystick::set_cooperative_level(void* hwnd, unsigned long level)
{
    pimpl_->set_cooperative_level(static_cast< ::HWND>(hwnd), level);
}

std::pair<direct_input::object_info_iterator,direct_input::object_info_iterator>
direct_input_joystick::objects(
    direct_input::object_type::values flags)
{
    return pimpl_->objects(flags);
}

std::pair<direct_input::object_info_iterator,direct_input::object_info_iterator>
direct_input_joystick::objects()
{
    return pimpl_->objects(DIDFT_ALL);
}

std::pair<long,long> direct_input_joystick::get_range(unsigned long type)
{
    return pimpl_->get_range(type);
}

void
direct_input_joystick::set_range(unsigned long type, long min_val, long max_val)
{
    pimpl_->set_range(type, min_val, max_val);
}

unsigned long direct_input_joystick::get_deadzone(unsigned long type)
{
    return pimpl_->get_dword_property(DIPROP_DEADZONE, type);
}

void direct_input_joystick::set_deadzone(unsigned long type, unsigned long val)
{
    pimpl_->set_dword_property(DIPROP_DEADZONE, type, val);
}

void direct_input_joystick::acquire()
{
    pimpl_->acquire();
}

void direct_input_joystick::unacquire()
{
    pimpl_->unacquire();
}

void direct_input_joystick::get_state(direct_input::joystick_state& state)
{
    pimpl_->get_state(state);
}

} } // End namespaces input, hamigaki.
