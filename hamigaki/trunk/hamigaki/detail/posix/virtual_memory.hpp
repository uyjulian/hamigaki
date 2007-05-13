// virtual_memory.hpp: a wrapper class for POSIX virtual memory

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_POSIX_VIRTUAL_MEMORY_HPP
#define HAMIGAKI_DETAIL_POSIX_VIRTUAL_MEMORY_HPP

#include <hamigaki/detail/posix/file.hpp>
#include <sys/mman.h>

namespace hamigaki { namespace detail { namespace posix {

class virtual_memory : boost::noncopyable
{
public:
    explicit virtual_memory(
            std::size_t size, int mode=PROT_READ|PROT_WRITE)
        : ptr_(::mmap(0, size, mode, MAP_PRIVATE,
            posix::file("/dev/zero", O_RDONLY).get(), 0))
        , size_(size)
    {
        if (ptr_ == MAP_FAILED)
            throw std::runtime_error("cannot allocate virtual memory");
    }

    ~virtual_memory()
    {
        ::munmap(ptr_, size_);
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

    void protect(int mode)
    {
        if (::mprotect(ptr_, size_, mode) == -1)
            throw std::runtime_error("failed mprotect()");
    }

private:
    void* ptr_;
    std::size_t size_;
};

} } } // End namespaces posix, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_POSIX_VIRTUAL_MEMORY_HPP
