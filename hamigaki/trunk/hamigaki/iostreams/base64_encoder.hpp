//  base64_encode_sink.hpp: Base64 encoder device

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_IOSTREAMS_BASE64_ENCODER_HPP
#define HAMIGAKI_IOSTREAMS_BASE64_ENCODER_HPP

#include <hamigaki/iostreams/arbitrary_positional_facade.hpp>
#include <boost/iostreams/detail/closer.hpp>
#include <boost/iostreams/write.hpp>
#include <boost/integer.hpp>

namespace hamigaki { namespace iostreams {

template<class Sink>
class base64_encode_sink
    : public arbitrary_positional_facade<base64_encode_sink<Sink>, char, 3>
{
    friend core_access;

    typedef typename boost::uint_t<24>::fast uint24_t;

public:
    typedef char char_type;

    struct category
        : public boost::iostreams::output
        , public boost::iostreams::device_tag
        , public boost::iostreams::closable_tag
    {};

    explicit base64_encode_sink(const Sink& sink) : sink_(sink) {}

private:
    Sink sink_;

    static uint24_t char_to_uint24(char c)
    {
        return static_cast<uint24_t>(static_cast<unsigned char>(c) & 0xFF);
    }

    static void encode(char* dst, const char* src)
    {
        const char table[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        uint24_t tmp =
            (char_to_uint24(src[0]) << 16) |
            (char_to_uint24(src[1]) <<  8) |
            (char_to_uint24(src[2])      ) ;

        dst[0] = table[(tmp >> 18) & 0x3F];
        dst[1] = table[(tmp >> 12) & 0x3F];
        dst[2] = table[(tmp >>  6) & 0x3F];
        dst[3] = table[(tmp      ) & 0x3F];
    }

    std::streamsize write_blocks(const char* s, std::streamsize n)
    {
        for (int i = 0; i < n; ++i)
        {
            char buf[4];
            encode(buf, s);
            boost::iostreams::write(sink_, buf, sizeof(buf));
            s += 3;
        }
        return n;
    }

    void close_with_flush(const char* s, std::streamsize n)
    {
        bool nothrow = false;
        boost::iostreams::detail::
            external_closer<Sink> close_sink(sink_, BOOST_IOS::out, nothrow);

        try
        {
            if (n != 0)
            {
                char src[3] = { 0, 0, 0 };
                std::copy(s, s+n, &src[0]);

                char buf[4];
                encode(buf, src);

                if (n == 1)
                    buf[2] = '=';
                buf[3] = '=';

                boost::iostreams::write(sink_, buf, sizeof(buf));
            }
        }
        catch (...)
        {
            nothrow = true;
            throw;
        }
    }
};

template<class Sink>
inline base64_encode_sink<Sink> base64_encoded(const Sink& sink)
{
    return base64_encode_sink<Sink>(sink);
}

} } // End namespaces iostreams, hamigaki.

#endif // HAMIGAKI_IOSTREAMS_BASE64_ENCODER_HPP
