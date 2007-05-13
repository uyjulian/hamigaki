// virtual_memory.hpp: a wrapper class for virtual memory

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_VIRTUAL_MEMORY_HPP
#define HAMIGAKI_DETAIL_VIRTUAL_MEMORY_HPP

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/detail/windows/virtual_memory.hpp>
#elif defined(BOOST_HAS_UNISTD_H)
    #include <hamigaki/detail/posix/virtual_memory.hpp>
#else
    #error "Sorry, unsupported architecture"
#endif

namespace hamigaki { namespace detail {

#if defined(BOOST_WINDOWS)
class virtual_memory : windows::virtual_memory
{
private:
    typedef windows::virtual_memory base_type;

public:
    enum protect_mode
    {
        read = 1,
        write = 2,
        execute = 4
    };

    explicit virtual_memory(std::size_t size) : base_type(size)
    {
    }

    using base_type::address;
    using base_type::size;
    using base_type::flush_icache;

    void protect(int mode)
    {
        ::DWORD m;
        if ((mode & execute) != 0)
        {
            if ((mode & write) != 0)
                m = PAGE_EXECUTE_READWRITE;
            else if ((mode & read) != 0)
                m = PAGE_EXECUTE_READ;
            else
                m = PAGE_EXECUTE;
        }
        else
        {
            if ((mode & write) != 0)
                m = PAGE_READWRITE;
            else if ((mode & read) != 0)
                m = PAGE_READONLY;
            else
                m = PAGE_NOACCESS;
        }

        base_type::protect(m);
    }
};
#elif defined(BOOST_HAS_UNISTD_H)
class virtual_memory : posix::virtual_memory
{
private:
    typedef posix::virtual_memory base_type;

public:
    enum protect_mode
    {
        read = PROT_READ,
        write = PROT_WRITE,
        execute = PROT_EXEC
    };

    explicit virtual_memory(std::size_t size) : base_type(size)
    {
    }

    using base_type::address;
    using base_type::size;

    void flush_icache()
    {
        // TODO
    }

    void protect(int mode)
    {
        base_type::protect(mode != 0 ? mode : PROT_NONE);
    }
};
#endif

} } // End namespaces detail, hamigaki.

#endif // HAMIGAKI_DETAIL_VIRTUAL_MEMORY_HPP
