// first_iterator.hpp: an iterator over "first" of elements of some sequence

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#ifndef HAMIGAKI_ITERATOR_FIRST_ITERATOR_HPP
#define HAMIGAKI_ITERATOR_FIRST_ITERATOR_HPP

#include <hamigaki/type_traits/member_access_traits.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <functional>
#include <iterator>

namespace hamigaki
{

namespace detail
{

template<typename Iterator>
struct select_first
    : std::unary_function<
        typename std::iterator_traits<Iterator>::value_type,
        typename boost::mpl::if_<
            typename boost::is_reference<
                typename std::iterator_traits<Iterator>::reference
            >::type,
            typename member_access_traits<
                typename std::iterator_traits<Iterator>::reference,
                typename std::iterator_traits<Iterator>::value_type::first_type
            >::reference,
            typename std::iterator_traits<Iterator>::value_type::first_type
        >::type
    >
{
    typename select_first::result_type
    operator()(typename std::iterator_traits<Iterator>::reference x) const
    {
        return x.first;
    }
};

} // namespace detail

template <class Iterator>
class first_iterator :
    public boost::transform_iterator<detail::select_first<Iterator>, Iterator>
{
    typedef detail::select_first<Iterator> function_type;
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
