// ostream_iterator.hpp: refinement of std::ostream_iterator

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iterator for library home page.

#ifndef HAMIGAKI_ITERATOR_OSTREAM_ITERATOR_HPP
#define HAMIGAKI_ITERATOR_OSTREAM_ITERATOR_HPP

#include <boost/shared_ptr.hpp>
#include <iosfwd>
#include <iterator>

namespace hamigaki
{

template <class T, class CharT=char, class Traits=std::char_traits<CharT> >
class ostream_iterator :
    public std::iterator<std::output_iterator_tag,void,void,void,void>
{
public:
    typedef CharT char_type;
    typedef Traits traits_type;
    typedef std::basic_ostream<CharT,Traits> ostream_type;

    ostream_iterator(ostream_type& s) : out_stream(&s), delim(0)
    {
    }

    ostream_iterator(ostream_type& s, const CharT* delimiter)
        : out_stream(&s), delim(delimiter), is_first(new bool(true))
    {
    }

    ostream_iterator<T,CharT,Traits>& operator=(const T& value)
    {
        if (delim)
        {
            if (*is_first)
                *is_first = false;
            else
                *out_stream << delim;
        }
        *out_stream << value;

        return *this;
    }

    ostream_iterator<T,CharT,Traits>& operator*()
    {
        return *this;
    }

    ostream_iterator<T,CharT,Traits>& operator++()
    {
        return *this;
    }

    ostream_iterator<T,CharT,Traits>& operator++(int)
    {
        return *this;
    }

private:
    ostream_type* out_stream;
    const CharT* delim;
    boost::shared_ptr<bool> is_first;
};

} // namespace hamigaki

#endif // HAMIGAKI_ITERATOR_OSTREAM_ITERATOR_HPP
