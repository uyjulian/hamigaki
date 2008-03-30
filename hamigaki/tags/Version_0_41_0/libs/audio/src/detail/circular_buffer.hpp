// circular_buffer.hpp: a tiny circular_buffer class

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_CIRCULAR_BUFFER_HPP
#define HAMIGAKI_AUDIO_DETAIL_CIRCULAR_BUFFER_HPP

#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <cstring>

namespace hamigaki { namespace audio { namespace detail {

// Note:
// My circular_buffer<T> is not copyable
// The element type T must be POD

template<class T>
class circular_buffer : private boost::noncopyable
{
public:
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef std::size_t size_type;
    typedef T* iterator; // dummy
    typedef std::pair<pointer,size_type> array_range;
    typedef size_type capacity_type;

    explicit circular_buffer(capacity_type capacity)
        : buffer_(new T[capacity]), capacity_(capacity)
        , pos_(0), size_(0)
    {
    }

    ~circular_buffer()
    {
        delete[] buffer_;
    }

    iterator end()
    {
        return 0; // dummy
    }

    array_range array_one()
    {
        return array_range(buffer_+pos_, (std::min)(size_, capacity_-pos_));
    }

    array_range array_two()
    {
        size_type end_pos = pos_ + size_;
        if (end_pos > capacity_)
            return array_range(buffer_, end_pos - capacity_);
        else
            return array_range(buffer_, 0);
    }

    size_type size() const
    {
        return size_;
    }

    bool empty() const
    {
        return size_ == 0;
    }

    bool full() const
    {
        return size_ == capacity_;
    }

    capacity_type capacity() const
    {
        return capacity_;
    }

    void rresize(size_type new_size)
    {
        BOOST_ASSERT(new_size <= size_);

        pos_ += (size_-new_size);
        pos_ %= capacity_;
        size_ = new_size;
    }

    void insert(iterator /*pos*/, const T* first, const T* last)
    {
        std::size_t n = last-first;
        BOOST_ASSERT(n <= capacity_ - size_);

        size_type end_pos = pos_ + size_;
        if (end_pos < capacity_)
        {
            size_type amt = (std::min)(capacity_-end_pos, n);
            std::memcpy(buffer_ + end_pos, first, sizeof(T)*amt);
            first += amt;
            size_ += amt;
            end_pos = pos_ + size_;
        }

        size_type amt = last-first;
        if (amt != 0)
        {
            std::memcpy(buffer_ + end_pos - capacity_, first, sizeof(T)*amt);
            size_ += amt;
        }
    }

    void clear()
    {
        pos_ = 0;
        size_ = 0;
    }

private:
    T* buffer_;
    capacity_type capacity_;
    size_type pos_;
    size_type size_;
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_CIRCULAR_BUFFER_HPP
