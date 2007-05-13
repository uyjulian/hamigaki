// stereo.hpp: stereo adaptor

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_STEREO_HPP
#define HAMIGAKI_AUDIO_STEREO_HPP

#include <hamigaki/iostreams/arbitrary_positional_facade.hpp>
#include <hamigaki/iostreams/catable.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/close.hpp>
#include <boost/iostreams/positioning.hpp>
#include <boost/iostreams/read.hpp>
#include <cstring>

namespace hamigaki { namespace audio {

template<class Source>
class stereophony
    : public hamigaki::iostreams::
        arbitrary_positional_facade<
            stereophony<Source>,
            typename boost::iostreams::char_type_of<Source>::type,
            255
        >
{
    friend class hamigaki::iostreams::core_access;

public:
    typedef typename boost::iostreams::
        char_type_of<Source>::type char_type;

    struct category
        : public boost::iostreams::input
        , public boost::iostreams::device_tag
        , public boost::iostreams::closable_tag
    {};

    explicit stereophony(const Source& src, unsigned channels=2)
        : stereophony<Source>::arbitrary_positional_facade_(channels)
        , src_(src), channels_(channels)
    {
    }

    void close()
    {
        boost::iostreams::close(src_, BOOST_IOS::in);
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

    std::streamsize read_blocks(char_type* s, std::streamsize n)
    {
        if (channels_ == 1)
            return boost::iostreams::read(src_, s, n);

        std::streamsize total = 0;
        for (std::streamsize i = 0; i < n; ++i)
        {
            std::streamsize amt = boost::iostreams::read(src_, s, 1);
            if (amt == -1)
                break;

            std::fill_n(s+1, channels_-1, *s);
            s += channels_;
            total += channels_;
        }

        return (total != 0) ? total : -1;
    }
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
