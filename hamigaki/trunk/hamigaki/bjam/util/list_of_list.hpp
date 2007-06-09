// list.hpp: list of list

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM_UTIL_LIST_OF_LIST_HPP
#define HAMIGAKI_BJAM_UTIL_LIST_OF_LIST_HPP

#include <hamigaki/bjam/util/list.hpp>

#if defined(BOOST_SPIRIT_DEBUG)
    #include <ostream>
#endif

namespace hamigaki { namespace bjam {

class list_of_list
{
public:
    typedef string_list value_type;
    typedef const value_type& const_reference;
    typedef const value_type* const_pointer;
    typedef std::size_t size_type;

    static const size_type static_max_size = 9;

    list_of_list() : size_(0)
    {
    }

    bool empty() const
    {
        return size_ == 0;
    }

    size_type size() const
    {
        return size_;
    }

    const value_type& operator[](size_type n) const
    {
        if (n < size_)
            return lists_[n];
        else
            return empty_;
    }

    void push_back(const value_type& x)
    {
        if (size_ < static_max_size)
            lists_[size_++] = x;
    }

    list_of_list& operator+=(const value_type& x)
    {
        this->push_back(x);
        return *this;
    }

    void swap(list_of_list& rhs)
    {
        std::swap(size_, rhs.size_);

        for (std::size_t i = 0; i < static_max_size; ++i)
            lists_[i].swap(rhs.lists_[i]);
    }

    void clear()
    {
        for (std::size_t i = 0; i < size_; ++i)
            lists_[i].clear();
        size_ = 0;
    }

private:
    size_type size_;
    value_type lists_[static_max_size];
    value_type empty_;
};

#if defined(BOOST_SPIRIT_DEBUG)
inline std::ostream& operator<<(std::ostream& os, const list_of_list& x)
{
    for (std::size_t i = 0, size = x.size(); i < size; ++i)
    {
        if (i != 0)
            os << " : ";
        os << x[i];
    }
    return os;
}
#endif

} } // End namespaces bjam, hamigaki.

#endif // HAMIGAKI_BJAM_UTIL_LIST_OF_LIST_HPP
