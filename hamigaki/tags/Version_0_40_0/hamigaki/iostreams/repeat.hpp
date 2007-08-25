// repeat.hpp: repeat view

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_REPEAT_HPP
#define HAMIGAKI_IOSTREAMS_REPEAT_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/adapter/direct_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/seek.hpp>
#include <boost/iostreams/traits.hpp>

namespace hamigaki { namespace iostreams {

template<class Source>
class repetition
{
private:
    typedef typename
        boost::iostreams::select<
            boost::iostreams::is_direct<Source>,
                boost::iostreams::detail::direct_adapter<Source>,
            boost::iostreams::else_,
                Source
        >::type value_type;

public:
    typedef typename boost::iostreams::
        char_type_of<value_type>::type char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag {};

    repetition(const Source& src, int count)
        : src_(src), count_(count)
    {
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        if (count_ == 0)
            return -1;

        std::streamsize total = 0;
        while (n > 0)
        {
            std::streamsize amt = boost::iostreams::read(src_, s, n);
            if (amt == -1)
            {
                if (count_ != -1)
                {
                    if (--count_ == 0)
                        break;
                }
                boost::iostreams::seek(src_, 0, BOOST_IOS::beg);
            }
            else
            {
                total += amt;
                s += amt;
                n -= amt;
            }
        }
        return (total != 0) ? total : -1;
    }

    void close()
    {
        boost::iostreams::close(src_, BOOST_IOS::in);
    }

private:
    value_type src_;
    int count_;
};

template<class Source>
inline repetition<Source>
repeat(const Source& src, int count)
{
    return repetition<Source>(src, count);
}

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_REPEAT_HPP
