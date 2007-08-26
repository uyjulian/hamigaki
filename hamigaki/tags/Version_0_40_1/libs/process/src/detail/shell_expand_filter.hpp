// shell_expand_filter.hpp: shell command expansion filter

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/process for library home page.

#ifndef HAMIGAKI_PROCESS_DETAIL_SHELL_EXPAND_FILTER_HPP
#define HAMIGAKI_PROCESS_DETAIL_SHELL_EXPAND_FILTER_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/read.hpp>
#include <string>

namespace hamigaki { namespace process { namespace detail {

class shell_expand_filter
{
public:
    typedef char char_type;

    struct category
        : boost::iostreams::input
        , boost::iostreams::filter_tag
    {};

    shell_expand_filter() : prev_(std::char_traits<char>::eof())
    {
    }

    template<class Source>
    int get(Source& src)
    {
        typedef std::char_traits<char> traits;

        if (!traits::eq_int_type(prev_, traits::eof()))
        {
            int i = prev_;
            prev_ = traits::eof();
            return i;
        }

        int i = boost::iostreams::get(src);
        if (traits::eq_int_type(i, traits::eof()))
            return i;

        char c = traits::to_char_type(i);
        if (c == '\r')
        {
            prev_ = boost::iostreams::get(src);
            if (traits::eq_int_type(prev_, traits::eof()))
                return prev_;

            c = traits::to_char_type(prev_);
            if (c == '\n')
            {
                prev_ = boost::iostreams::get(src);
                if (traits::eq_int_type(prev_, traits::eof()))
                    return prev_;
                else
                    return ' ';
            }
            else
                return ' ';
        }
        else if (c == '\n')
        {
            prev_ = boost::iostreams::get(src);
            if (traits::eq_int_type(prev_, traits::eof()))
                return prev_;
            else
                return ' ';
        }
        else
            return i;
    }

private:
    int prev_;
};

} } } // End namespaces detail, process, hamigaki.

#endif // HAMIGAKI_PROCESS_DETAIL_SHELL_EXPAND_FILTER_HPP
