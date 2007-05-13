// cdecl_thunk.hpp: an instance thunk for __cdecl functions for i386

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_I386_CDECL_THUNK_HPP
#define HAMIGAKI_DETAIL_I386_CDECL_THUNK_HPP

#include <hamigaki/detail/i386/func_ptr_cast.hpp>
#include <boost/assert.hpp>
#include <cstring>

namespace hamigaki { namespace detail {

template<std::size_t Argc>
class cdecl_thunk
{
public:
    void set_instance(void* func, void* ptr)
    {
        std::size_t pos = 0;

        // push ebp
        entry_[pos++] = 0x55;

        // mov ebp,esp
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xEC;

        for (std::size_t i = 0; i < Argc; ++i)
        {
            // mov eax,dword ptr [ebp + 4*(1+Argc-i)]
            entry_[pos++] = 0x8B;
            entry_[pos++] = 0x45;
            entry_[pos++] = 4*(1+Argc-i);

            // push eax
            entry_[pos++] = 0x50;
        }

        // push "ptr"
        entry_[pos++] = 0x68;
        std::memcpy(&entry_[pos], &ptr, 4);
        pos += 4;

        // call "func"
        boost::int32_t rel32 =
            reinterpret_cast<boost::int32_t>(func) -
            reinterpret_cast<boost::int32_t>(&entry_[pos+5]);
        entry_[pos++] = 0xE8;
        std::memcpy(&entry_[pos], &rel32, 4);
        pos += 4;

        // add esp, 4*(1+Argc)
        entry_[pos++] = 0x83;
        entry_[pos++] = 0xC4;
        entry_[pos++] = 4*(1+Argc);

        // pop ebp
        entry_[pos++] = 0x5D;

        // ret
        entry_[pos++] = 0xC3;

        BOOST_ASSERT(pos == sizeof(entry_));
    }

    void* address() { return entry_; }

    template<class T>
    void copy_address(T*& ptr)
    {
        ptr = ::hamigaki::detail::func_ptr_cast<T*>(address());
    }

private:
    boost::uint8_t entry_[18+4*Argc];
};

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_I386_CDECL_THUNK_HPP
