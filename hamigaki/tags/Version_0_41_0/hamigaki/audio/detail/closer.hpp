// closer.hpp: a tiny clone of boost::iostreams::detail::external_closer

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_CLOSER_HPP
#define HAMIGAKI_AUDIO_DETAIL_CLOSER_HPP

#include <boost/noncopyable.hpp>

namespace hamigaki { namespace audio { namespace detail {

template<class Device>
class device_closer : private boost::noncopyable
{
public:
    device_closer(Device& dev, bool& nothrow) : dev_(dev), nothrow_(nothrow)
    {
    }

    ~device_closer()
    {
        try
        {
            dev_.close();
        }
        catch (...)
        {
            if (!nothrow_)
            {
                nothrow_ = true;
                throw;
            }
        }
    }

private:
    Device& dev_;
    bool& nothrow_;
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_CLOSER_HPP
