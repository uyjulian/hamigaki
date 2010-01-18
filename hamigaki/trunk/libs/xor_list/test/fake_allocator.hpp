// fake_allocator.hpp: fake pointer and allocator

// Copyright Takeshi Mouri 2010.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/xor_list for library home page.

#ifndef HAMIGAKI_FAKE_ALLOCATOR_HPP
#define HAMIGAKI_FAKE_ALLOCATOR_HPP

#include <memory>

template<class T>
class fake_pointer
{
public:
    fake_pointer(T* p) : ptr_(p)
    {
    }

    T& operator*() const
    {
        return *ptr_;
    }

    T* operator->() const
    {
        return ptr_;
    }

private:
    T* ptr_;
};

template<>
class fake_pointer<void>
{
public:
    fake_pointer(void* p) : ptr_(p)
    {
    }

private:
    void* ptr_;
};

template<class T>
class const_fake_pointer
{
public:
    const_fake_pointer(const T* p) : ptr_(p)
    {
    }

    const_fake_pointer(fake_pointer<T> p) : ptr_(&*p)
    {
    }

    const T& operator*() const
    {
        return *ptr_;
    }

    const T* operator->() const
    {
        return ptr_;
    }

private:
    const T* ptr_;
};

template<>
class const_fake_pointer<void>
{
public:
    const_fake_pointer(const void* p) : ptr_(p)
    {
    }

private:
    const void* ptr_;
};

template<class T>
struct fake_allocator;

template<>
struct fake_allocator<void>
{
    typedef fake_pointer<void> pointer;
    typedef const_fake_pointer<void> const_pointer;
    typedef void value_type;

    template<class U>
    struct rebind { typedef fake_allocator<U> other; };
};

template<class T>
struct fake_allocator
{
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef fake_pointer<T> pointer;
    typedef const_fake_pointer<T> const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template<class U>
    struct rebind { typedef fake_allocator<U> other; };

    fake_allocator() throw() {}
    fake_allocator(const fake_allocator&) throw() {}

    template<class U>
    fake_allocator(const fake_allocator<U>&) throw() {}

    ~fake_allocator() throw() {}

    pointer address(reference x) const
    {
        return pointer(&x);
    }

    const_pointer address(const_reference x) const
    {
        return const_pointer(&x);
    }

    pointer allocate(size_type n, fake_allocator<void>::const_pointer hint = 0)
    {
        return pointer(std::allocator<T>().allocate(n));
    }

    void deallocate(pointer p, size_type n)
    {
        std::allocator<T>().deallocate(&*p, n);
    }

    size_type max_size() const throw()
    {
        return std::allocator<T>().max_size();
    }

    void construct(pointer p, const T& val)
    {
        new(static_cast<void*>(&*p)) T(val);
    }

    void destroy(pointer p)
    {
        p->~T();
    }
};

template<class T, class U>
bool operator==(const fake_allocator<T>&, const fake_allocator<U>&) throw()
{
    return true;
}

template<class T, class U>
bool operator!=(const fake_allocator<T>&, const fake_allocator<U>&) throw()
{
    return false;
}

#endif // HAMIGAKI_FAKE_ALLOCATOR_HPP
