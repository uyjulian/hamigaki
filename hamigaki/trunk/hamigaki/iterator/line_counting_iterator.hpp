// line_counting_iterator.hpp: a line-counting adaptor for iterators

// Copyright Takeshi Mouri 2007, 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#ifndef HAMIGAKI_ITERATOR_LINE_COUNTING_ITERATOR_HPP
#define HAMIGAKI_ITERATOR_LINE_COUNTING_ITERATOR_HPP

#include <hamigaki/static_widen.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/assert.hpp>

namespace hamigaki
{

namespace impl
{

template<class Iterator>
struct select_line_counting_iter_category
{
    typedef typename boost::iterator_category<Iterator>::type original_category;

    typedef typename boost::mpl::if_<
        typename boost::is_convertible<
            original_category,
            std::random_access_iterator_tag
        >::type,
        std::bidirectional_iterator_tag,
        original_category
    >::type type;
};

} // namespace impl

template<class Iterator>
class line_counting_iterator
    : public boost::iterator_adaptor<
        line_counting_iterator<Iterator>,
        Iterator,
        boost::use_default,
        typename impl::select_line_counting_iter_category<Iterator>::type
    >
{
    friend class boost::iterator_core_access;

private:
    typedef line_counting_iterator<Iterator> self_type;
    typedef typename self_type::iterator_adaptor_ adaptor_type;

public:
    line_counting_iterator() : adaptor_type(Iterator()), line_(-1), nl_()
    {
    }

    explicit line_counting_iterator(Iterator it)
        : adaptor_type(it), line_(-1)
        , nl_(static_widen<typename adaptor_type::value_type,'\n'>::value)
    {
    }

    line_counting_iterator(Iterator it, int line)
        : adaptor_type(it), line_(line)
        , nl_(static_widen<typename adaptor_type::value_type,'\n'>::value)
    {
    }

    line_counting_iterator(Iterator it, int line,
        typename adaptor_type::value_type nl
    )
        : adaptor_type(it), line_(line), nl_(nl)
    {
    }

    int line() const
    {
        return line_;
    }

private:
    int line_;
    typename adaptor_type::value_type nl_;

    void increment()
    {
        BOOST_ASSERT(line_ >= 0);

        Iterator& p = this->base_reference();
        if (*p == nl_)
            ++line_;
        ++p;
    }

    void decrement()
    {
        BOOST_ASSERT(line_ >= 0);

        Iterator& p = this->base_reference();
        --p;
        if (*p == nl_)
            --line_;
    }
};

} // namespace hamigaki

#endif // HAMIGAKI_ITERATOR_LINE_COUNTING_ITERATOR_HPP
