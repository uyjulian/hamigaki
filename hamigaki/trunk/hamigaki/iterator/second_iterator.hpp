//  second_iterator.hpp: an iterator over "second" of elements of some sequence

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_ITERATOR_SECOND_ITERATOR_HPP
#define HAMIGAKI_ITERATOR_SECOND_ITERATOR_HPP

#include <boost/iterator/transform_iterator.hpp>
#include <functional>

namespace hamigaki
{

namespace detail
{

template<typename T>
struct select_second : std::unary_function<T, typename T::second_type>
{
    typename T::second_type operator()(const T& x) const
    {
        return x.second;
    }
};

} // namespace detail

template <class Iterator>
class second_iterator :
    public boost::transform_iterator<
        detail::select_second<typename Iterator::value_type>,
        Iterator
    >
{
    typedef detail::select_second<
        typename Iterator::value_type> function_type;

    typedef boost::transform_iterator<function_type,Iterator> super_t;

public:
    second_iterator(){}

    second_iterator(const Iterator& x)
        : super_t(x, function_type())
    {
    }
};

template <class Iterator>
inline second_iterator<Iterator> make_second_iterator(const Iterator& x)
{
    return second_iterator<Iterator>(x);
}

} // namespace hamigaki

#endif // HAMIGAKI_ITERATOR_SECOND_ITERATOR_HPP
