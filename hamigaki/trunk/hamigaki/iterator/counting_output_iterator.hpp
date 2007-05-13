// counting_output_iterator.hpp: the output version of counting_iterator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#ifndef HAMIGAKI_ITERATOR_COUNTING_OUTPUT_ITERATOR_HPP
#define HAMIGAKI_ITERATOR_COUNTING_OUTPUT_ITERATOR_HPP

#include <iterator>

namespace hamigaki
{

namespace detail
{

struct null_output_proxy
{
    template<class T>
    null_output_proxy& operator=(const T&)
    {
        return *this; 
    }
};

} // namespace detail

template <class Incrementable>
class counting_output_iterator :
    public std::iterator<std::output_iterator_tag,void,void,void,void>
{
public:
    counting_output_iterator() : base_()
    {
    }

    explicit counting_output_iterator(const Incrementable& x)
        : base_(x)
    {
    }

    detail::null_output_proxy operator*()
    {
        return detail::null_output_proxy();
    }

    counting_output_iterator<Incrementable>& operator++()
    {
        ++base_;
        return *this;
    }

    counting_output_iterator<Incrementable> operator++(int)
    {
        counting_output_iterator<Incrementable> tmp(*this);
        ++base_;
        return tmp;
    }

    const Incrementable& base() const
    {
        return base_;
    }

private:
    Incrementable base_;
};

template<class Incrementable>
inline counting_output_iterator<Incrementable>
make_counting_output_iterator(const Incrementable& x)
{
    return counting_output_iterator<Incrementable>(x);
}

} // namespace hamigaki

#endif // HAMIGAKI_ITERATOR_COUNTING_OUTPUT_ITERATOR_HPP
