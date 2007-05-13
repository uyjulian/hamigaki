// direct_sound_notify.hpp: DirectSoundNotify Object

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_DIRECT_SOUND_NOTIFY_HPP
#define HAMIGAKI_AUDIO_DETAIL_DIRECT_SOUND_NOTIFY_HPP

#include <hamigaki/audio/direct_sound.hpp>
#include <boost/noncopyable.hpp>
#include <dsound.h>

namespace hamigaki { namespace audio { namespace detail {

class direct_sound_notify : boost::noncopyable
{
public:
    template<class Interface>
    explicit direct_sound_notify(Interface* buf_ptr)
    {
        void* tmp;
        direct_sound_error::check(buf_ptr->QueryInterface(iid(), &tmp));
        ptr_ = static_cast< ::IDirectSoundNotify*>(tmp);
    }

    ~direct_sound_notify()
    {
        ptr_->Release();
    }

    void set(const ::DSBPOSITIONNOTIFY* data, ::DWORD size)
    {
        direct_sound_error::check(
            ptr_->SetNotificationPositions(size, data));
    }

private:
    ::IDirectSoundNotify* ptr_;

    static const ::GUID iid()
    {
        // IID_IDirectSoundNotify
        const ::GUID tmp =
        {
            0xB0210783, 0x89CD, 0x11D0,
            { 0xAF, 0x08, 0x00, 0xA0, 0xC9, 0x25, 0xCD, 0x16 }
        };
        return tmp;
    }
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_DIRECT_SOUND_NOTIFY_HPP
