// optional_iterator.hpp: iterator with "empty" state

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#ifndef HAMIGAKI_ITERATOR_OPTIONAL_ITERATOR_HPP
#define HAMIGAKI_ITERATOR_OPTIONAL_ITERATOR_HPP

#include <boost/iterator/iterator_adaptor.hpp>

namespace hamigaki
{

template<class Iterator>
class optional_iterator
    : public boost::iterator_adaptor<
        optional_iterator<Iterator>,
        Iterator
    >
{
    friend class boost::iterator_core_access;

private:
    typedef optional_iterator<Iterator> self_type;
    typedef typename self_type::iterator_adaptor_ adaptor_type;

public:
    optional_iterator() : valid_(false)
    {
    }

    optional_iterator(Iterator it) : adaptor_type(it), valid_(true)
    {
    }

private:
    bool valid_;

    bool equal(const self_type& rhs) const
    {
        if (valid_)
        {
            if (rhs.valid_)
                return this->base() == rhs.base();
            else
                return false;
        }
        else
            return !rhs.valid_;
    }
};

} // namespace hamigaki

#endif // HAMIGAKI_ITERATOR_OPTIONAL_ITERATOR_HPP
