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

extern const ::DIDATAFORMAT keyboard_data_format;
extern const ::DIDATAFORMAT joystick_data_format;

const unsigned long all_devices = DIEDFL_ALLDEVICES;
const unsigned long attached_only = DIEDFL_ATTACHEDONLY;
const unsigned long force_feedback_only = DIEDFL_FORCEFEEDBACK;

const unsigned long exclusive_level = DISCL_EXCLUSIVE;
const unsigned long nonexclusive_level = DISCL_NONEXCLUSIVE;
const unsigned long foreground_level = DISCL_FOREGROUND;
const unsigned long background_level = DISCL_BACKGROUND;

const unsigned long ff_actuator = DIDOI_FFACTUATOR;
const unsigned long ff_effect_trigger = DIDOI_FFEFFECTTRIGGER;
const unsigned long polled = DIDOI_POLLED;
const unsigned long aspect_position = DIDOI_ASPECTPOSITION;
const unsigned long aspect_velocity = DIDOI_ASPECTVELOCITY;
const unsigned long aspect_accel = DIDOI_ASPECTACCEL;
const unsigned long aspect_force = DIDOI_ASPECTFORCE;

namespace
{

object_info convert_obj_info(const ::DIDEVICEOBJECTINSTANCEA& ddoi)
{
    object_info info;

    info.type_guid = uuid(ddoi.guidType);
    info.offset = ddoi.dwOfs;
    info.type = object_type::from_dword(ddoi.dwType);
    info.flags = ddoi.dwFlags;
    info.name = ddoi.tszName;
    info.max_force = ddoi.dwFFMaxForce;
    info.force_resolution = ddoi.dwFFForceResolution;
    info.collection_number = ddoi.wCollectionNumber;
    info.designator_index = ddoi.wDesignatorIndex;
    info.usage_page = ddoi.wUsagePage;
    info.usage = ddoi.wUsage;
    info.dimension = ddoi.dwDimension;
    info.exponent = ddoi.wExponent;

    return info;
}

unsigned long get_dword_property(
    ::IDirectInputDevice2A* pimpl, unsigned long how, unsigned long key,
    const ::GUID& guid)
{
    ::DIPROPDWORD data;
    std::memset(&data, 0, sizeof(data));
    data.diph.dwSize = sizeof(data);
    data.diph.dwHeaderSize = sizeof(::DIPROPHEADER);
    data.diph.dwObj = key;
    data.diph.dwHow = how;

    ::HRESULT res = pimpl->GetProperty(guid, &data.diph);
    if (FAILED(res))
        throw direct_input_error(res, "IDirectInputDevice2A::GetProperty()");

    return data.dwData;
}

void set_dword_property(
    ::IDirectInputDevice2A* pimpl, unsigned long how, unsigned long key,
    const ::GUID& guid, unsigned long val)
{
    ::DIPROPDWORD data;
    std::memset(&data, 0, sizeof(data));
    data.diph.dwSize = sizeof(data);
    data.diph.dwHeaderSize = sizeof(::DIPROPHEADER);
    data.diph.dwObj = key;
    data.diph.dwHow = how;
    data.dwData = val;

    ::HRESULT res = pimpl->SetProperty(guid, &data.diph);
    if (FAILED(res))
        throw direct_input_error(res, "IDirectInputDevice2A::SetProperty()");
}

} // namespace

device_object::device_object(
    const boost::shared_ptr< ::IDirectInputDevice2A>& p,
    unsigned long how, unsigned long key
)
    : pimpl_(p), how_(how), key_(key)
{
}

device_object::~device_object()
{
}

object_info device_object::info() const
{
    ::DIDEVICEOBJECTINSTANCEA tmp;

    ::HRESULT res = pimpl_->GetObjectInfo(&tmp, key_, how_);
    if (FAILED(res))
        throw direct_input_error(res, "IDirectInputDevice2A::GetObjectInfo()");

    return convert_obj_info(tmp);
}

std::pair<long,long> device_object::range()
{
    ::DIPROPRANGE data;
    std::memset(&data, 0, sizeof(data));
    data.diph.dwSize = sizeof(data);
    data.diph.dwHeaderSize = sizeof(::DIPROPHEADER);
    data.diph.dwObj = key_;
    data.diph.dwHow = how_;

    ::HRESULT res = pimpl_->GetProperty(DIPROP_RANGE, &data.diph);
    if (FAILED(res))
        throw direct_input_error(res, "IDirectInputDevice2A::GetProperty()");

    return std::make_pair(data.lMin, data.lMax);
}

void device_object::range(long min_val, long max_val)
{
    ::DIPROPRANGE data;
    std::memset(&data, 0, sizeof(data));
    data.diph.dwSize = sizeof(data);
    data.diph.dwHeaderSize = sizeof(::DIPROPHEADER);
    data.diph.dwObj = key_;
    data.diph.dwHow = how_;
    data.lMin = min_val;
    data.lMax = max_val;

    ::HRESULT res = pimpl_->SetProperty(DIPROP_RANGE, &data.diph);
    if (FAILED(res))
        throw direct_input_error(res, "IDirectInputDevice2A::SetProperty()");
}

unsigned long device_object::dead_zone()
{
    return get_dword_property(pimpl_.get(), how_, key_, DIPROP_DEADZONE);
}

void device_object::dead_zone(unsigned long val)
{
    set_dword_property(pimpl_.get(), how_, key_, DIPROP_DEADZONE, val);
}

unsigned long device_object::saturation()
{
    return get_dword_property(pimpl_.get(), how_, key_, DIPROP_SATURATION);
}

void device_object::saturation(unsigned long val)
{
    set_dword_property(pimpl_.get(), how_, key_, DIPROP_SATURATION, val);
}

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

    ::IDirectInputDevice2A* create_device(const ::GUID& guid)
    {
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

direct_input_keyboard
direct_input_manager::create_keyboard_device(const uuid& instance)
{
    ::GUID guid;
    instance.copy(guid);
    return direct_input_keyboard(pimpl_->create_device(guid));
}

direct_input_keyboard direct_input_manager::create_keyboard_device()
{
    const ::GUID sys_keyboard_guid =
    { 0x6F1D2B61,0xD5A0,0x11CF, {0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00} };

    return direct_input_keyboard(pimpl_->create_device(sys_keyboard_guid));
}

direct_input_joystick
direct_input_manager::create_joystick_device(const uuid& instance)
{
    ::GUID guid;
    instance.copy(guid);
    return direct_input_joystick(pimpl_->create_device(guid));
}


class direct_input::device_impl
{
public:
    explicit device_impl(const boost::shared_ptr< ::IDirectInputDevice2A>& ptr)
        : pimpl_(ptr)
    {
    }

    ~device_impl()
    {
        try
        {
            unacquire();
        }
        catch (...)
        {
        }
    }

    void set_data_format(const ::DIDATAFORMAT& fmt)
    {
        ::HRESULT res = pimpl_->SetDataFormat(&fmt);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::SetDataFormat()");
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
                boost::bind(&device_impl::enum_objects, this, _1, flags)
            ),
            object_info_iterator()
        );
    }

    direct_input::device_object object(unsigned long how, unsigned long key)
    {
        return direct_input::device_object(pimpl_, how, key);
    }

    bool auto_center()
    {
        unsigned long dw =
            direct_input::get_dword_property(
                pimpl_.get(), DIPH_DEVICE, 0, DIPROP_AUTOCENTER);
        return dw != DIPROPAUTOCENTER_OFF;
    }

    void auto_center(bool val)
    {
        unsigned long dw = val ? DIPROPAUTOCENTER_ON : DIPROPAUTOCENTER_OFF;
        direct_input::set_dword_property(
            pimpl_.get(), DIPH_DEVICE, 0, DIPROP_AUTOCENTER, dw);
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

    void get_state(void* buf, std::size_t size)
    {
        ::HRESULT res = pimpl_->Poll();
        if ((res == DIERR_INPUTLOST) || (res == DIERR_NOTACQUIRED))
        {
            acquire();
            res = pimpl_->Poll();
        }
        if (FAILED(res))
            throw direct_input_error(res, "IDirectInputDevice2A::Poll()");

        res = pimpl_->GetDeviceState(size, buf);
        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::GetDeviceState()");
        }
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

            self.yield(direct_input::convert_obj_info(*lpddoi));

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
            pimpl_->EnumObjects(
                &device_impl::enum_objects_callback, &self, flags);

        if (FAILED(res))
        {
            throw direct_input_error(
                res, "IDirectInputDevice2A::EnumObjects()");
        }

        self.exit();
        HAMIGAKI_COROUTINE_UNREACHABLE_RETURN(direct_input::object_info())
    }
};


direct_input_keyboard::direct_input_keyboard(::IDirectInputDevice2A* p)
{
    boost::shared_ptr< ::IDirectInputDevice2A> tmp(p, com_release());

    pimpl_.reset(new direct_input::device_impl(tmp));
    pimpl_->set_data_format(direct_input::keyboard_data_format);
}

direct_input_keyboard::~direct_input_keyboard()
{
}

void
direct_input_keyboard::set_cooperative_level(void* hwnd, unsigned long level)
{
    pimpl_->set_cooperative_level(static_cast< ::HWND>(hwnd), level);
}

std::pair<direct_input::object_info_iterator,direct_input::object_info_iterator>
direct_input_keyboard::objects(const direct_input::object_type& type)
{
    return pimpl_->objects(type.to_dword());
}

std::pair<direct_input::object_info_iterator,direct_input::object_info_iterator>
direct_input_keyboard::objects()
{
    return pimpl_->objects(DIDFT_ALL);
}

direct_input::device_object direct_input_keyboard::object(unsigned long offset)
{
    return pimpl_->object(DIPH_BYOFFSET, offset);
}

direct_input::device_object
direct_input_keyboard::object(const direct_input::object_type& type)
{
    return pimpl_->object(DIPH_BYID, type.to_dword());
}

void direct_input_keyboard::acquire()
{
    pimpl_->acquire();
}

void direct_input_keyboard::unacquire()
{
    pimpl_->unacquire();
}

void direct_input_keyboard::get_state(direct_input::keyboard_state& state)
{
    pimpl_->get_state(state.elems, direct_input::keyboard_state::static_size);
}


direct_input_joystick::direct_input_joystick(::IDirectInputDevice2A* p)
{
    boost::shared_ptr< ::IDirectInputDevice2A> tmp(p, com_release());

    pimpl_.reset(new direct_input::device_impl(tmp));
    pimpl_->set_data_format(direct_input::joystick_data_format);
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
direct_input_joystick::objects(const direct_input::object_type& type)
{
    return pimpl_->objects(type.to_dword());
}

std::pair<direct_input::object_info_iterator,direct_input::object_info_iterator>
direct_input_joystick::objects()
{
    return pimpl_->objects(DIDFT_ALL);
}

direct_input::device_object direct_input_joystick::object(unsigned long offset)
{
    return pimpl_->object(DIPH_BYOFFSET, offset);
}

direct_input::device_object
direct_input_joystick::object(const direct_input::object_type& type)
{
    return pimpl_->object(DIPH_BYID, type.to_dword());
}

bool direct_input_joystick::auto_center()
{
    return pimpl_->auto_center();
}

void direct_input_joystick::auto_center(bool val)
{
    pimpl_->auto_center(val);
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
    ::DIJOYSTATE2 tmp;
    pimpl_->get_state(&tmp, sizeof(tmp));

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

} } // End namespaces input, hamigaki.
