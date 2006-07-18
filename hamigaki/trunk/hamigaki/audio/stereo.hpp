//  stereo.hpp: stereo adaptor

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_AUDIO_STEREO_HPP
#define HAMIGAKI_AUDIO_STEREO_HPP

#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <cstring>

namespace hamigaki { namespace audio {

template<class Source>
class stereophony
{
public:
    typedef typename boost::iostreams::
        char_type_of<Source>::type char_type;

    struct category
        : public boost::iostreams::input
        , public boost::iostreams::device_tag
    {};

    explicit stereophony(const Source& src, unsigned channels=2)
        : src_(src), channels_(channels)
    {
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        if (channels_ == 1)
            return boost::iostreams::read(src_, s, n);

        if (n <= 0)
            return -1;

        if (n % channels_ != 0)
            throw BOOST_IOSTREAMS_FAILURE("invalid read size");

        const std::streamsize count = n / channels_;
        for (std::streamsize i = 0; i < count; ++i)
        {
            std::streamsize amt = boost::iostreams::read(src_, s, 1);
            if (amt == -1)
                return i ? i : -1;

            std::fill_n(s+1, channels_-1, *s);
            s += channels_;
        }

        return n;
    }

    std::streampos seek(boost::iostreams::stream_offset, BOOST_IOS::seekdir)
    {
        return -1;
    }

    unsigned channels() const
    {
        return channels_;
    }

private:
    Source src_;
    unsigned channels_;
};

template<class Source>
inline stereophony<Source>
stereo(const Source& src, unsigned channels=2)
{
    return stereophony<Source>(src, channels);
}

} } // End namespaces audio, hamigaki.

HAMIGAKI_IOSTREAMS_CATABLE(hamigaki::audio::stereophony, 1)

#endif // HAMIGAKI_AUDIO_STEREO_HPP
