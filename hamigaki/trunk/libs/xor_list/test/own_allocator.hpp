// own_allocator.hpp: simple "not equal" allocator

// Copyright Takeshi Mouri 2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/xor_list for library home page.

#ifndef HAMIGAKI_OWN_ALLOCATOR_HPP
#define HAMIGAKI_OWN_ALLOCATOR_HPP

#include <boost/type_traits/alignment_of.hpp>
#include <boost/aligned_storage.hpp>
#include <cassert>
#include <cstddef>
#include <memory>

template<class T>
class own_allocator;

template<>
class own_allocator<void>
{
public:
    typedef void* pointer;
    typedef const void* const_pointer;
    typedef void value_type;

    template<class U>
    struct rebind { typedef own_allocator<U> other; };
};

template<class T>
class own_allocator
{
private:
    struct node_type
    {
        void* owner;
        typename boost::aligned_storage<
            sizeof(T),
            boost::alignment_of<T>::value
        >::type value;
    };

public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template<class U>
    struct rebind { typedef own_allocator<U> other; };

    own_allocator() throw()
        : owner_(0)
    {
    }

    own_allocator(const own_allocator& x) throw()
        : owner_(x.owner_)
    {
    }

    template<class U>
    own_allocator(const own_allocator<U>& x) throw()
        : owner_(x.owner())
    {
    }

    ~own_allocator() throw() {}

    pointer address(reference x) const
    {
        return &x;
    }

    const_pointer address(const_reference x) const
    {
        return &x;
    }

    pointer allocate(
        size_type n,
        own_allocator<void>::const_pointer hint = 0)
    {
        void* p = std::allocator<char>().allocate(
            sizeof(node_type) + sizeof(T)*(n-1)
        );
        node_type* node = static_cast<node_type*>(p);
        if (!owner_)
            owner_ = this;
        node->owner = owner_;
        return static_cast<T*>(static_cast<void*>(&(node->value)));
    }

    void deallocate(pointer p, size_type n)
    {
        void* top =
            static_cast<char*>(static_cast<void*>(p)) -
            offsetof(node_type, value);
        node_type* node = static_cast<node_type*>(top);
        assert(node->owner == owner_);
        std::allocator<char>().deallocate(
            static_cast<char*>(top), sizeof(node_type)+sizeof(T)*(n-1) );
    }

    size_type max_size() const throw()
    {
        return std::allocator<T>().max_size();
    }

    void construct(pointer p, const T& val)
    {
        new(static_cast<void*>(p)) T(val);
    }

    void destroy(pointer p)
    {
        p->~T();
    }

    void* owner() const
    {
        return owner_;
    }

private:
    void* owner_;
};

template<class T, class U>
bool
operator==(const own_allocator<T>& lhs, const own_allocator<U>& rhs) throw()
{
    return lhs.owner() == rhs.owner();
}

template<class T, class U>
bool
operator!=(const own_allocator<T>& lhs, const own_allocator<U>& rhs) throw()
{
    return !(lhs == rhs);
}

#endif // HAMIGAKI_FAKE_ALLOCATOR_HPP
