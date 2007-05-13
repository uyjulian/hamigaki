// buffer.hpp: refinement of boost::iostreams::detail::buffer

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

// Original Copyright
// ===========================================================================>
// (C) Copyright Jonathan Turkanis 2005.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)

// See http://www.boost.org/libs/iostreams for documentation.
// <===========================================================================

#ifndef HAMIGAKI_IOSTREAMS_DETAIL_BUFFER_HPP
#define HAMIGAKI_IOSTREAMS_DETAIL_BUFFER_HPP

#include <boost/noncopyable.hpp>
#include <cstring>
#include <vector>

namespace hamigaki { namespace iostreams { namespace detail {

template <
    class CharT,
    class Allocator = std::allocator<CharT>
>
class buffer : private boost::noncopyable
{
private:
    typedef std::vector<CharT,Allocator> buffer_type;
    typedef typename buffer_type::size_type size_type;

public:
    explicit buffer(std::streamsize buffer_size)
        : buffer_(static_cast<size_type>(buffer_size))
    {
    }

    void resize(std::streamsize buffer_size)
    {
        buffer_.resize(static_cast<size_type>(buffer_size));
    }

    CharT* data()
    {
        return &buffer_[0];
    }

    const CharT* data() const
    {
        return &buffer_[0];
    }

    CharT* begin()
    {
        return &buffer_[0];
    }

    const CharT* begin() const
    {
        return &buffer_[0];
    }

    CharT* end()
    {
        return &buffer_[0] + buffer_.size();
    }

    const CharT* end() const
    {
        return &buffer_[0] + buffer_.size();
    }

    std::streamsize size() const
    {
        return static_cast<std::streamsize>(buffer_.size());
    }

    CharT*& ptr()
    {
        return ptr_;
    }

    const CharT*& ptr() const
    {
        return ptr_;
    }

    CharT*& eptr()
    {
        return eptr_;
    }

    const CharT*& eptr() const
    {
        return eptr_;
    }

    void set(std::streamsize ptr, std::streamsize end)
    { 
        ptr_ = data() + ptr; 
        eptr_ = data() + end; 
    }

    template<class Sink>
    bool flush(Sink& dest) 
    {
        std::streamsize amt = static_cast<std::streamsize>(eptr_ - ptr_);
        std::streamsize result = boost::iostreams::write(dest, ptr_, amt);
        if (result < amt)
        {
            std::memmove(
                this->data(),
                ptr_ + result,
                sizeof(CharT) * (amt - result)
            );
        }

        set(0, amt - result);
        return result != 0;
    }

private:
    buffer_type buffer_;
    CharT* ptr_;
    CharT* eptr_;
};

} } } // End namespaces detail, iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_DETAIL_BUFFER_HPP
