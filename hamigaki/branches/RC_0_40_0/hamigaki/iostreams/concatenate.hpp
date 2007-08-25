// concatenate.hpp: concatenation of devices

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_IOSTREAMS_CONCATENATE_HPP
#define HAMIGAKI_IOSTREAMS_CONCATENATE_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/adapter/direct_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>

namespace hamigaki { namespace iostreams {

// Source1 and Source2 must be blocking
template<class Source1, class Source2>
class concatenation
{
private:
    typedef typename
        boost::iostreams::select<
            boost::iostreams::is_direct<Source1>,
                boost::iostreams::detail::direct_adapter<Source1>,
            boost::iostreams::else_,
                Source1
        >::type source1_type;

    typedef typename
        boost::iostreams::select<
            boost::iostreams::is_direct<Source2>,
                boost::iostreams::detail::direct_adapter<Source2>,
            boost::iostreams::else_,
                Source2
        >::type source2_type;

public:
    typedef typename boost::iostreams::
        char_type_of<source1_type>::type char_type;

    struct category :
        boost::iostreams::input,
        boost::iostreams::device_tag,
        boost::iostreams::closable_tag {};

    concatenation(const Source1& src1, const Source2& src2)
        : src1_(src1), src2_(src2), eof1_(false)
    {
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        if (!eof1_)
        {
            std::streamsize amt = boost::iostreams::read(src1_, s, n);
            if (amt != -1)
            {
                total += amt;
                s += amt;
                n -= amt;
            }

            if (amt < n)
                eof1_ = true;
        }

        if (n != 0)
        {
            std::streamsize amt = boost::iostreams::read(src2_, s, n);
            if (amt != -1)
                total += amt;
        }

        return total != 0 ? total : -1;
    }

    void close()
    {
        boost::iostreams::close(src1_, BOOST_IOS::in);
        boost::iostreams::close(src2_, BOOST_IOS::in);
    }

private:
    typedef typename boost::iostreams::
        char_type_of<source2_type>::type second_char_type;
    BOOST_STATIC_ASSERT((boost::is_same<char_type, second_char_type>::value));

    source1_type src1_;
    source2_type src2_;
    bool eof1_;
};

template<class Source1, class Source2>
inline concatenation<Source1,Source2>
concatenate(const Source1& src1, const Source2& src2)
{
    return concatenation<Source1,Source2>(src1, src2);
}

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_CONCATENATE_HPP
