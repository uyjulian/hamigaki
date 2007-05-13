// virtual_memory.hpp: a wrapper class for VirtualAlloc()

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_VIRTUAL_MEMORY_HPP
#define HAMIGAKI_DETAIL_WINDOWS_VIRTUAL_MEMORY_HPP

#include <boost/noncopyable.hpp>
#include <stdexcept>
#include <cstddef>
#include <windows.h>

namespace hamigaki { namespace detail { namespace windows {

class virtual_memory : boost::noncopyable
{
public:
    explicit virtual_memory(
            std::size_t size, ::DWORD mode=PAGE_READWRITE)
        : ptr_(::VirtualAlloc(0, size, MEM_COMMIT|MEM_RESERVE, mode))
        , size_(size)
    {
        if (ptr_ == 0)
            throw std::runtime_error("cannot allocate virtual memory");
    }

    ~virtual_memory()
    {
        ::VirtualFree(ptr_, 0, MEM_DECOMMIT|MEM_RELEASE);
    }

    void* address()
    {
        return ptr_;
    }

    const void* address() const
    {
        return ptr_;
    }

    std::size_t size() const
    {
        return size_;
    }

    void flush_icache()
    {
        ::FlushInstructionCache(::GetCurrentProcess(), ptr_, size_);
    }

    ::DWORD protect(::DWORD mode)
    {
        ::DWORD old = 0;
        if (!::VirtualProtect(ptr_, size_, mode, &old))
            throw std::runtime_error("failed VirtualProtect");
        return old;
    }

private:
    void* ptr_;
    std::size_t size_;
};

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_VIRTUAL_MEMORY_HPP
