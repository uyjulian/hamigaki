// asio_drivers.cpp: ASIO devices

// Copyright Takeshi Mouri 2006-2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#define HAMIGAKI_AUDIO_SOURCE
#define NOMINMAX
#include <hamigaki/audio/asio/drivers.hpp>

#include <hamigaki/detail/windows/registry.hpp>

namespace hamigaki { namespace audio { namespace asio {

HAMIGAKI_AUDIO_DECL std::vector<driver_info> driver_list()
{
    using namespace hamigaki::detail::windows;

    std::string asio_key_name("SOFTWARE\\ASIO");
    std::vector<driver_info> drivers;
    try
    {
        registry_key asio_key(HKEY_LOCAL_MACHINE, asio_key_name, KEY_READ);

        for (registry_key_iterator i(asio_key), end; i != end; ++i)
        {
            std::string sub_key_name(asio_key_name);
            sub_key_name += '\\';
            sub_key_name += *i;

            registry_key key(HKEY_LOCAL_MACHINE, sub_key_name, KEY_READ);
            driver_info info;
            info.clsid = uuid(key.get_value("CLSID").c_str());
            info.name = key.get_value("Description");
            drivers.push_back(info);
        }
    }
    catch (const std::exception&)
    {
    }
    return drivers;
}

} } } // End namespaces asio, audio, hamigaki.
