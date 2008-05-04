// wide_adaptor.hpp: an adaptor for making wide character stream

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_WIDE_ADAPTOR_HPP
#define HAMIGAKI_AUDIO_WIDE_ADAPTOR_HPP

#include <hamigaki/audio/detail/wide_adaptor_char_float.hpp>
#include <hamigaki/audio/detail/wide_adaptor_char_int.hpp>
#include <hamigaki/audio/detail/wide_adaptor_float_float.hpp>
#include <hamigaki/iostreams/traits.hpp>
#include <boost/iostreams/detail/select.hpp>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/shared_ptr.hpp>
#include <limits>

namespace hamigaki { namespace audio {

namespace detail
{

template<class CharT, std::size_t Bits>
struct is_int_exact_t
{
    static const bool value =
        std::numeric_limits<CharT>::is_integer &&
        std::numeric_limits<CharT>::is_signed &&
        std::numeric_limits<CharT>::digits+1 == Bits;
};

} // namespace detail

template<class CharT, class Device>
class wide_adaptor
{
    typedef typename boost::iostreams::
        char_type_of<Device>::type base_char_type;

    typedef typename
        boost::iostreams::select<
            boost::is_floating_point<base_char_type>,
                detail::wide_adaptor_float_float<CharT,Device>,

            boost::is_same<CharT,boost::int_t<32> >,
                detail::wide_adaptor_char_int<32,Device>,
            boost::is_same<CharT,boost::int_t<24> >,
                detail::wide_adaptor_char_int<24,Device>,
            boost::is_same<CharT,boost::int_t<16> >,
                detail::wide_adaptor_char_int<16,Device>,

            detail::is_int_exact_t<CharT,32>,
                detail::wide_adaptor_char_int<32,Device,CharT>,
            detail::is_int_exact_t<CharT,24>,
                detail::wide_adaptor_char_int<24,Device,CharT>,
            detail::is_int_exact_t<CharT,16>,
                detail::wide_adaptor_char_int<16,Device,CharT>,

            boost::iostreams::else_,
                detail::wide_adaptor_char_float<CharT,Device>
        >::type impl_type;

public:
    typedef typename impl_type::char_type char_type;

    struct category
        : boost::iostreams::mode_of<Device>::type
        , boost::iostreams::device_tag
        , boost::iostreams::closable_tag
        , boost::iostreams::optimally_buffered_tag
    {};

    explicit wide_adaptor(const Device& dev)
        : pimpl_(
            new impl_type(dev, boost::iostreams::optimal_buffer_size(dev)))
    {
    }

    wide_adaptor(const Device& dev, std::streamsize buffer_size)
        : pimpl_(new impl_type(dev, buffer_size))
    {
    }

    void close()
    {
        this->close_impl(
            boost::is_convertible<
                typename boost::iostreams::mode_of<Device>::type,
                boost::iostreams::output
            >()
        );
    }

    void close(BOOST_IOS::openmode which)
    {
        pimpl_->close(which);
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        return pimpl_->read(s, n);
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        return pimpl_->write(s, n);
    }

    std::streampos seek(iostreams::stream_offset off, BOOST_IOS::seekdir way)
    {
        return this->seek_impl(off, way,
            boost::is_convertible<
                typename boost::iostreams::mode_of<Device>::type,
                boost::iostreams::output
            >()
        );
    }

    std::streamsize optimal_buffer_size() const
    {
        return pimpl_->optimal_buffer_size();
    }

private:
    boost::shared_ptr<impl_type> pimpl_;

    void close_impl(const boost::false_type&)
    {
        pimpl_->close(BOOST_IOS::in);
    }

    void close_impl(const boost::true_type&)
    {
        pimpl_->close(BOOST_IOS::out);
    }

    std::streampos seek_impl(
        iostreams::stream_offset off, BOOST_IOS::seekdir way,
        const boost::false_type&)
    {
        return pimpl_->seek(off, way, BOOST_IOS::in);
    }

    std::streampos seek_impl(
        iostreams::stream_offset off, BOOST_IOS::seekdir way,
        const boost::true_type&)
    {
        return pimpl_->seek(off, way, BOOST_IOS::out);
    }
};

template<class CharT, class Device>
inline wide_adaptor<CharT, Device>
widen(const Device& dev)
{
    return wide_adaptor<CharT, Device>(dev);
}

template<class CharT, class Device>
inline wide_adaptor<CharT, Device>
widen(const Device& dev, std::streamsize buffer_size)
{
    return wide_adaptor<CharT, Device>(dev, buffer_size);
}

} } // End namespaces audio, hamigaki.

#endif // HAMIGAKI_AUDIO_WIDE_ADAPTOR_HPP
