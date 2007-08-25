// virtual_device.hpp: virtual audio device adaptor

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_VIRTUAL_DEVICE_HPP
#define HAMIGAKI_AUDIO_VIRTUAL_DEVICE_HPP

#include <hamigaki/audio/pcm_format.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>

namespace hamigaki { namespace audio {

template<typename Device>
class virtual_device : public Device
{
public:
    typedef typename boost::iostreams::char_type_of<Device>::type char_type;

    struct category :
        Device::category,
        pcm_format_tag {};

    virtual_device(const Device& dev, const pcm_format& fmt)
        : Device(dev), format_(fmt)
    {
    }

    pcm_format format() const
    {
        return format_;
    }

private:
    pcm_format format_;
};

template<typename Device>
inline virtual_device<Device>
make_virtual_device(const Device& dev, const pcm_format& fmt)
{
    return virtual_device<Device>(dev, fmt);
}

} } // End namespaces audio, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::audio::virtual_device, 1)

#endif // HAMIGAKI_AUDIO_VIRTUAL_DEVICE_HPP
