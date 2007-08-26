// pcm_wave_format.hpp: PCMWAVEFORMAT structure

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_PCM_WAVE_FORMAT_HPP
#define HAMIGAKI_AUDIO_DETAIL_PCM_WAVE_FORMAT_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace audio { namespace detail {

struct pcm_wave_format
{
    boost::uint16_t format_tag;
    boost::uint16_t channels;
    boost::uint32_t samples_per_sec;
    boost::uint32_t avg_bytes_per_sec;
    boost::uint16_t block_align;
    boost::uint16_t bits_per_sample;
};

} } } // End namespaces detail, audio, hamigaki.


namespace hamigaki {

template<>
struct struct_traits<hamigaki::audio::detail::pcm_wave_format>
{
private:
    typedef hamigaki::audio::detail::pcm_wave_format self;

public:
    typedef boost::mpl::list<
        member<self, boost::uint16_t, &self::format_tag, little>,
        member<self, boost::uint16_t, &self::channels, little>,
        member<self, boost::uint32_t, &self::samples_per_sec, little>,
        member<self, boost::uint32_t, &self::avg_bytes_per_sec, little>,
        member<self, boost::uint16_t, &self::block_align, little>,
        member<self, boost::uint16_t, &self::bits_per_sample, little>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_AUDIO_DETAIL_PCM_WAVE_FORMAT_HPP
