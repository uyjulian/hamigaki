// sample_format.hpp: sample format type

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_SAMPLE_FORMAT_HPP
#define HAMIGAKI_AUDIO_SAMPLE_FORMAT_HPP

#include <boost/iostreams/detail/dispatch.hpp>
#include <boost/iostreams/detail/wrap_unwrap.hpp>
#include <boost/iostreams/operations_fwd.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/if.hpp>
#include <iosfwd>

namespace hamigaki { namespace audio {

enum sample_format_type
{
    uint8,
    int8,
    int_le16,
    int_be16,
    int_le24,
    int_be24,
    int_le32,
    int_be32,
    int_a4_le16,
    int_a4_be16,
    int_a4_le18,
    int_a4_be18,
    int_a4_le20,
    int_a4_be20,
    int_a4_le24,
    int_a4_be24,
    float_le32,
    float_be32,
    float_le64,
    float_be64,
    mu_law,
    a_law,
};

inline std::streamsize sample_size(sample_format_type type)
{
    const std::streamsize table[] =
    {
        1, 1, 2, 2, 3, 3, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 8, 8,
        1, 1
    };
    BOOST_STATIC_ASSERT(sizeof(table)/sizeof(table[0]) == 22);

    return table[type];
}

inline int sample_bits(sample_format_type type)
{
    const int table[] =
    {
        8, 8, 16, 16, 24, 24, 32, 32,
        16, 16, 18, 18, 20, 20, 24, 24,
        32, 32, 64, 64,
        8, 8
    };
    BOOST_STATIC_ASSERT(sizeof(table)/sizeof(table[0]) == 22);

    return table[type];
}

struct sample_format_tag : virtual boost::iostreams::any_tag {};
struct pcm_format_tag : virtual boost::iostreams::any_tag {};

namespace detail
{

template<typename T>
struct sample_format_of_impl;

template<typename T>
struct sample_format_of_impl
    : boost::mpl::if_<
        boost::iostreams::detail::is_custom<T>,
        boost::iostreams::operations<T>,
        sample_format_of_impl<
            typename boost::iostreams::detail::dispatch<
                T, sample_format_tag, pcm_format_tag
            >::type
        >
    >::type {};

template<>
struct sample_format_of_impl<sample_format_tag>
{
    template<typename T>
    static sample_format_type sample_format_of(const T& t)
    {
        return t.sample_format();
    }
};

template<>
struct sample_format_of_impl<pcm_format_tag>
{
    template<typename T>
    static sample_format_type sample_format_of(const T& t)
    {
        return t.format().type;
    }
};

} // namespace detail

template<typename T>
inline sample_format_type sample_format_of(const T& t)
{
    typedef detail::sample_format_of_impl<T> impl;
    return impl::sample_format_of(boost::iostreams::detail::unwrap(t));
}

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_PCM_FORMAT_HPP
