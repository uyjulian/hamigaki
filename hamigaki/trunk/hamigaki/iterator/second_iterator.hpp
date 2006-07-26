//  second_iterator.hpp: an iterator over "second" of elements of some sequence

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_ITERATOR_FIRST_ITERATOR_HPP
#define HAMIGAKI_ITERATOR_FIRST_ITERATOR_HPP

#include <hamigaki/type_traits/member_access_traits.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <functional>
#include <iterator>

namespace hamigaki
{

namespace detail
{

template<typename Iterator>
struct select_second
    : std::unary_function<
        typename std::iterator_traits<Iterator>::value_type,
        typename member_access_traits<
            typename std::iterator_traits<Iterator>::reference,
            typename std::iterator_traits<Iterator>::value_type::second_type
        >::reference
    >
{
    typename select_second::result_type
    operator()(typename std::iterator_traits<Iterator>::reference x) const
    {
        return x.second;
    }
};

} // namespace detail

template <class Iterator>
class second_iterator :
    public boost::transform_iterator<detail::select_second<Iterator>, Iterator>
{
    typedef detail::select_second<Iterator> function_type;
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

#endif // HAMIGAKI_ITERATOR_FIRST_ITERATOR_HPP
