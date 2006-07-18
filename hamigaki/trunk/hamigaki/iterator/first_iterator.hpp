//  first_iterator.hpp: an iterator over "first" of elements of some sequence

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_ITERATOR_FIRST_ITERATOR_HPP
#define HAMIGAKI_ITERATOR_FIRST_ITERATOR_HPP

#include <boost/iterator/transform_iterator.hpp>
#include <functional>

namespace hamigaki
{

namespace detail
{

template<typename T>
struct select_first : std::unary_function<T, typename T::first_type>
{
    typename T::first_type operator()(const T& x) const
    {
        return x.first;
    }
};

} // namespace detail

template <class Iterator>
class first_iterator :
    public boost::transform_iterator<
        detail::select_first<typename Iterator::value_type>,
        Iterator
    >
{
    typedef detail::select_first<
        typename Iterator::value_type> function_type;

    typedef boost::transform_iterator<function_type,Iterator> super_t;

public:
    first_iterator(){}

    first_iterator(const Iterator& x)
        : super_t(x, function_type())
    {
    }
};

template <class Iterator>
inline first_iterator<Iterator> make_first_iterator(const Iterator& x)
{
    return first_iterator<Iterator>(x);
}

} // namespace hamigaki

#endif // HAMIGAKI_ITERATOR_FIRST_ITERATOR_HPP
