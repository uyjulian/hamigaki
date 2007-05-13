// amplify.hpp: amplifier view

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_AMPLIFY_HPP
#define HAMIGAKI_AUDIO_AMPLIFY_HPP

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/detail/adapter/direct_adapter.hpp>
#include <boost/iostreams/detail/ios.hpp>
#include <boost/iostreams/detail/select.hpp>
#include <boost/iostreams/optimal_buffer_size.hpp>
#include <boost/iostreams/read.hpp>
#include <boost/iostreams/traits.hpp>
#include <algorithm>
#include <functional>

namespace hamigaki { namespace audio {

template<class Source>
class amplifier
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
        boost::iostreams::closable_tag,
        boost::iostreams::optimally_buffered_tag {};

    amplifier(const Source& src, float amp)
        : src_(src), amp_(amp)
    {
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        std::streamsize amt = boost::iostreams::read(src_, s, n);
        if (amt != -1)
        {
            // TODO: shall support non-floating type
            std::transform(
                s, s+amt, s,
                std::bind2nd(
                    std::multiplies<char_type>(),
                    static_cast<char_type>(amp_)
                )
            );
        }
        return amt;
    }

    void close()
    {
        boost::iostreams::close(src_, BOOST_IOS::in);
    }

    std::streamsize optimal_buffer_size() const
    {
        return boost::iostreams::optimal_buffer_size(src_);
    }

private:
    value_type src_;
    float amp_;
};

template<class Source>
inline amplifier<Source>
amplify(const Source& src, float amp)
{
    return amplifier<Source>(src, amp);
}

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_AMPLIFY_HPP
