// device_number.hpp: the device number class

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#ifndef HAMIGAKI_FILESYSTEM_DEVICE_NUMBER_HPP
#define HAMIGAKI_FILESYSTEM_DEVICE_NUMBER_HPP

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/operators.hpp>

#if defined(__unix__)
    #include <sys/types.h>

    #if defined(__hpux) || defined(__sgi) || defined(sun) || defined(__sun)
        #include <sys/mkdev.h>
    #elif !defined(__NetBSD__) && !defined(__OpenBSD__)
        #include <sys/sysmacros.h>
    #endif
#endif

namespace hamigaki { namespace filesystem {

struct device_number : boost::totally_ordered<device_number>
{
#if defined(__unix__)
    typedef ::dev_t native_type;
#else
    typedef boost::uint16_t native_type;
#endif

    boost::intmax_t major;
    boost::intmax_t minor;

    device_number()
    {
        major = 0;
        minor = 0;
    }

    device_number(boost::intmax_t maj, boost::intmax_t min)
    {
        major = maj;
        minor = min;
    }

    static device_number from_native(native_type n)
    {
#if defined(__unix__)
        return device_number(major(n), minor(n));
#else
        return device_number((n >> 8) & 0xFFu, n & 0xFFu);
#endif
    }

    native_type to_native() const
    {
#if defined(__unix__)
        return makedev(major, minor);
#else
        return static_cast<native_type>(
            ((static_cast<boost::uintmax_t>(major) & 0xFFu) << 8) |
            ((static_cast<boost::uintmax_t>(minor) & 0xFFu)     )
        );
#endif
    }

    bool operator<(const device_number& rhs) const
    {
        return
            (major < rhs.major) ||
            ((major == rhs.major) && (minor < rhs.minor));
    }

    bool operator==(const device_number& rhs) const
    {
        return (major == rhs.major) && (minor == rhs.minor);
    }
};

} } // End namespaces filesystem, hamigaki.

#endif // HAMIGAKI_FILESYSTEM_DEVICE_NUMBER_HPP
