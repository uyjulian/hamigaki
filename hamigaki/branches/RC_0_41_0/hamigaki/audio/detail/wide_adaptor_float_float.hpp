// wide_adaptor_char_float.hpp: floating point <-> floating point converter

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/audio for library home page.

#ifndef HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_FLOAT_FLOAT_HPP
#define HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_FLOAT_FLOAT_HPP

#include <hamigaki/iostreams/positioning.hpp>
#include <boost/iostreams/operations.hpp>
#include <vector>

namespace hamigaki { namespace audio { namespace detail {

template<class CharT, class Device>
class wide_adaptor_float_float
{
    typedef typename boost::iostreams::
        char_type_of<Device>::type base_char_type;

public:
    typedef CharT char_type;

    wide_adaptor_float_float(const Device& dev, std::streamsize buffer_size)
        : dev_(dev), buffer_(buffer_size)
    {
    }

    void close(BOOST_IOS::openmode which)
    { 
        boost::iostreams::close(dev_, which);
    }

    std::streamsize read(char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (total != n)
        {
            std::streamsize amt = read_once(s + total, n - total);
            if (amt == -1)
                break;
            total += amt;
        }

        return (total != 0) ? total : -1;
    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        std::streamsize total = 0;

        while (total != n)
            total += write_once(s + total, n - total);

        return total;
    }

    std::streampos seek(
        boost::iostreams::stream_offset off,
        BOOST_IOS::seekdir way, BOOST_IOS::openmode which)
    {
        return boost::iostreams::seek(dev_, off, way, which);
    }

    std::streamsize optimal_buffer_size() const
    {
        return buffer_.size();
    }

private:
    Device dev_;
    std::vector<base_char_type> buffer_;

    std::streamsize read_once(char_type* s, std::streamsize n)
    {
        std::streamsize count =
            (std::min)(n, static_cast<std::streamsize>(buffer_.size()));

        std::streamsize amt =
            boost::iostreams::read(dev_, &buffer_[0], count);
        if (amt == -1)
            return -1;

        for (std::streamsize i = 0; i < amt; ++i)
            s[i] = static_cast<char_type>(buffer_[i]);

        return amt;
    }

    std::streamsize write_once(const char_type* s, std::streamsize n)
    {
        std::streamsize count =
            (std::min)(n, static_cast<std::streamsize>(buffer_.size()));

        for (std::streamsize i = 0; i < count; ++i)
            buffer_[i] = static_cast<base_char_type>(s[i]);

        return boost::iostreams::write(dev_, &buffer_[0], count);
    }
};

} } } // End namespaces detail, audio, hamigaki.

#endif // HAMIGAKI_AUDIO_DETAIL_WIDE_ADAPTOR_FLOAT_FLOAT_HPP
