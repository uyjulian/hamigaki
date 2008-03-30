// stdcall_thunk.hpp: an instance thunk for __stdcall functions for i386

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_I386_STDCALL_THUNK_HPP
#define HAMIGAKI_DETAIL_I386_STDCALL_THUNK_HPP

#include <hamigaki/detail/i386/func_ptr_cast.hpp>
#include <boost/assert.hpp>
#include <cstring>

namespace hamigaki { namespace detail {

class stdcall_thunk
{
public:
    void set_instance(void* func, void* ptr)
    {
        std::size_t pos = 0;

        // push dword ptr [esp]
        entry_[pos++] = 0xFF;
        entry_[pos++] = 0x34;
        entry_[pos++] = 0x24;

        // mov dword ptr [esp+4], "ptr"
        entry_[pos++] = 0xC7;
        entry_[pos++] = 0x44;
        entry_[pos++] = 0x24;
        entry_[pos++] = 0x04;

        std::memcpy(&entry_[pos], &ptr, 4);
        pos += 4;

        // jmp "func"
        boost::int32_t rel32 =
            reinterpret_cast<boost::int32_t>(func) -
            reinterpret_cast<boost::int32_t>(&entry_[pos+5]);
        entry_[pos++] = 0xE9;
        std::memcpy(&entry_[pos], &rel32, 4);
        pos += 4;

        BOOST_ASSERT(pos == sizeof(entry_));
    }

    void* address() { return entry_; }

    template<class T>
    void copy_address(T*& ptr)
    {
        ptr = ::hamigaki::detail::func_ptr_cast<T*>(address());
    }

private:
    boost::uint8_t entry_[16];
};

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_I386_STDCALL_THUNK_HPP
