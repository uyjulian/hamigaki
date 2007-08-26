// drivers.hpp: ASIO driver list

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_ASIO_DRIVERS_HPP
#define HAMIGAKI_AUDIO_ASIO_DRIVERS_HPP

#include <hamigaki/audio/detail/config.hpp>
#include <hamigaki/uuid.hpp>
#include <string>
#include <vector>

#if defined(BOOST_WINDOWS) && !defined(__GNUC__)
#pragma comment(lib, "advapi32.lib")
#endif

namespace hamigaki { namespace audio { namespace asio {

struct driver_info
{
    uuid clsid;
    std::string name;
};

HAMIGAKI_AUDIO_DECL std::vector<driver_info> driver_list();

} } } // End namespaces asio, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_ASIO_DRIVERS_HPP
