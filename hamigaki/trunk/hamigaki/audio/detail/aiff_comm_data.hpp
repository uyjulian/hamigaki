// aiff_comm_data.hpp: AIFF COMM chunk data structure

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_AIFF_COMM_DATA_HPP
#define HAMIGAKI_AUDIO_DETAIL_AIFF_COMM_DATA_HPP

#include <hamigaki/binary/struct_traits.hpp>
#include <boost/mpl/list.hpp>
#include <boost/cstdint.hpp>

namespace hamigaki { namespace audio { namespace detail {

struct aiff_comm_data
{
    boost::int16_t num_channels;
    boost::int32_t num_sample_frames;
    boost::int16_t sample_size;
    boost::int16_t sample_rate_exp;
    boost::uint64_t sample_rate_mant;
};

} } } // End namespaces detail, audio, hamigaki.


namespace hamigaki {

template<>
struct struct_traits<hamigaki::audio::detail::aiff_comm_data>
{
private:
    typedef hamigaki::audio::detail::aiff_comm_data self;

public:
    typedef boost::mpl::list<
        member<self, boost::int16_t, &self::num_channels, big>,
        member<self, boost::int32_t, &self::num_sample_frames, big>,
        member<self, boost::int16_t, &self::sample_size, big>,
        member<self, boost::int16_t, &self::sample_rate_exp, big>,
        member<self, boost::uint64_t, &self::sample_rate_mant, big>
    > members;
};

} // namespace hamigaki

#endif // HAMIGAKI_AUDIO_DETAIL_AIFF_COMM_DATA_HPP
