// cdecl_thunk.hpp: an instance thunk for __cdecl functions for x64

// Copyright Takeshi Mouri 2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_X64_CDECL_THUNK_HPP
#define HAMIGAKI_DETAIL_X64_CDECL_THUNK_HPP

#include <hamigaki/detail/x64/func_ptr_cast.hpp>
#include <boost/assert.hpp>
#include <cstring>

namespace hamigaki { namespace detail {

template<std::size_t Argc>
class cdecl_thunk {};

template<>
class cdecl_thunk<0>
{
public:
    void set_instance(void* func, void* ptr)
    {
        std::size_t pos = 0;

        // mov rcx,"ptr"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB9;
        std::memcpy(&entry_[pos], &ptr, 8);
        pos += 8;

        // mov rax,"func"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB8;
        std::memcpy(&entry_[pos], &func, 8);
        pos += 8;

        // jmp rax
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xFF;
        entry_[pos++] = 0xE0;

        BOOST_ASSERT(pos == sizeof(entry_));
    }

    void* address() { return entry_; }

    template<class T>
    void copy_address(T*& ptr)
    {
        ptr = ::hamigaki::detail::func_ptr_cast<T*>(address());
    }

private:
    boost::uint8_t entry_[23];
};

template<>
class cdecl_thunk<1>
{
public:
    void set_instance(void* func, void* ptr)
    {
        std::size_t pos = 0;

        // mov rdx,rcx
        entry_[pos++] = 0x48;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xD1;

        // mov rcx,"ptr"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB9;
        std::memcpy(&entry_[pos], &ptr, 8);
        pos += 8;

        // mov rax,"func"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB8;
        std::memcpy(&entry_[pos], &func, 8);
        pos += 8;

        // jmp rax
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xFF;
        entry_[pos++] = 0xE0;

        BOOST_ASSERT(pos == sizeof(entry_));
    }

    void* address() { return entry_; }

    template<class T>
    void copy_address(T*& ptr)
    {
        ptr = ::hamigaki::detail::func_ptr_cast<T*>(address());
    }

private:
    boost::uint8_t entry_[26];
};

template<>
class cdecl_thunk<2>
{
public:
    void set_instance(void* func, void* ptr)
    {
        std::size_t pos = 0;

        // mov r8,rdx
        entry_[pos++] = 0x4C;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xC2;

        // mov rdx,rcx
        entry_[pos++] = 0x48;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xD1;

        // mov rcx,"ptr"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB9;
        std::memcpy(&entry_[pos], &ptr, 8);
        pos += 8;

        // mov rax,"func"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB8;
        std::memcpy(&entry_[pos], &func, 8);
        pos += 8;

        // jmp rax
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xFF;
        entry_[pos++] = 0xE0;

        BOOST_ASSERT(pos == sizeof(entry_));
    }

    void* address() { return entry_; }

    template<class T>
    void copy_address(T*& ptr)
    {
        ptr = ::hamigaki::detail::func_ptr_cast<T*>(address());
    }

private:
    boost::uint8_t entry_[29];
};

template<>
class cdecl_thunk<3>
{
public:
    void set_instance(void* func, void* ptr)
    {
        std::size_t pos = 0;

        // mov r9,r8
        entry_[pos++] = 0x4D;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xC8;

        // mov r8,rdx
        entry_[pos++] = 0x4C;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xC2;

        // mov rdx,rcx
        entry_[pos++] = 0x48;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xD1;

        // mov rcx,"ptr"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB9;
        std::memcpy(&entry_[pos], &ptr, 8);
        pos += 8;

        // mov rax,"func"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB8;
        std::memcpy(&entry_[pos], &func, 8);
        pos += 8;

        // jmp rax
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xFF;
        entry_[pos++] = 0xE0;

        BOOST_ASSERT(pos == sizeof(entry_));
    }

    void* address() { return entry_; }

    template<class T>
    void copy_address(T*& ptr)
    {
        ptr = ::hamigaki::detail::func_ptr_cast<T*>(address());
    }

private:
    boost::uint8_t entry_[32];
};

template<>
class cdecl_thunk<4>
{
public:
    void set_instance(void* func, void* ptr)
    {
        std::size_t pos = 0;

        // sub rsp,38h
        entry_[pos++] = 0x43;
        entry_[pos++] = 0x83;
        entry_[pos++] = 0xEC;
        entry_[pos++] = 0x38;

        // mov qword ptr [rsp+20h],r9
        entry_[pos++] = 0x4C;
        entry_[pos++] = 0x89;
        entry_[pos++] = 0x4C;
        entry_[pos++] = 0x24;
        entry_[pos++] = 0x20;

        // mov r9,r8
        entry_[pos++] = 0x4D;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xC8;

        // mov r8,rdx
        entry_[pos++] = 0x4C;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xC2;

        // mov rdx,rcx
        entry_[pos++] = 0x48;
        entry_[pos++] = 0x8B;
        entry_[pos++] = 0xD1;

        // mov rcx,"ptr"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB9;
        std::memcpy(&entry_[pos], &ptr, 8);
        pos += 8;

        // mov rax,"func"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB8;
        std::memcpy(&entry_[pos], &func, 8);
        pos += 8;

        // call rax
        entry_[pos++] = 0xFF;
        entry_[pos++] = 0xD0;

        // add rsp,38h
        entry_[pos++] = 0x48;
        entry_[pos++] = 0x83;
        entry_[pos++] = 0xC4;
        entry_[pos++] = 0x38;

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
    boost::uint8_t entry_[45];
};


class cdecl_thunk_double
{
public:
    void set_instance(void* func, void* ptr)
    {
        std::size_t pos = 0;

        // movapd xmm1,xmm0
        entry_[pos++] = 0x66;
        entry_[pos++] = 0x0F;
        entry_[pos++] = 0x28;
        entry_[pos++] = 0xC8;

        // mov rcx,"ptr"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB9;
        std::memcpy(&entry_[pos], &ptr, 8);
        pos += 8;

        // mov rax,"func"
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xB8;
        std::memcpy(&entry_[pos], &func, 8);
        pos += 8;

        // jmp rax
        entry_[pos++] = 0x48;
        entry_[pos++] = 0xFF;
        entry_[pos++] = 0xE0;

        BOOST_ASSERT(pos == sizeof(entry_));
    }

    void* address() { return entry_; }

    template<class T>
    void copy_address(T*& ptr)
    {
        ptr = ::hamigaki::detail::func_ptr_cast<T*>(address());
    }

private:
    boost::uint8_t entry_[27];
};

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_X64_CDECL_THUNK_HPP
