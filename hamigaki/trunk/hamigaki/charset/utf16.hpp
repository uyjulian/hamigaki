// utf16.hpp: utility for UTF-16

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#ifndef HAMIGAKI_CHARSET_UTF16_HPP
#define HAMIGAKI_CHARSET_UTF16_HPP

#include <hamigaki/binary/endian.hpp>
#include <hamigaki/charset/wstring.hpp>

namespace hamigaki { namespace charset {

namespace utf16_detail
{

template<std::size_t CharSize>
struct utf16_impl;

template<>
struct utf16_impl<2>
{
    template<hamigaki::endianness E>
    static std::string to_utf16(const wstring& ws)
    {
        std::size_t n = ws.size();

        std::string s;
        s.reserve(n*2);

        char buf[2];
        for (std::size_t i = 0; i < n; ++i)
        {
            hamigaki::encode_int<E,2>(buf,static_cast<boost::uint16_t>(ws[i]));
            s.push_back(buf[0]);
            s.push_back(buf[1]);
        }
        return s;
    }
};

template<>
struct utf16_impl<4>
{
    template<hamigaki::endianness E>
    static std::string to_utf16(const wstring& ws)
    {
        std::size_t n = ws.size();

        std::string s;
        s.reserve(n*2);

        char buf[2];
        for (std::size_t i = 0; i < n; ++i)
        {
            boost::uint32_t wc = static_cast<boost::uint32_t>(ws[i]);
            if ((wc & 0xFFFF0000) != 0)
            {
                wc -= 0x10000;
                boost::uint16_t u1 =
                    static_cast<boost::uint16_t>(0xD800 | (wc >> 10));
                boost::uint16_t u2 =
                    static_cast<boost::uint16_t>(0xDC00 | (wc & 0x3FF));

                hamigaki::encode_int<E,2>(buf, u1);
                s.push_back(buf[0]);
                s.push_back(buf[1]);

                hamigaki::encode_int<E,2>(buf, u2);
                s.push_back(buf[0]);
                s.push_back(buf[1]);
            }
            else
            {
                hamigaki::encode_int<E,2>(buf,static_cast<boost::uint16_t>(wc));
                s.push_back(buf[0]);
                s.push_back(buf[1]);
            }
        }
        return s;
    }
};

} // namespace utf16_detail

inline std::string to_utf16be(const wstring& ws)
{
    typedef utf16_detail::utf16_impl<sizeof(wchar_t)> impl_type;
    return impl_type::to_utf16<hamigaki::big>(ws);
}

inline std::string to_utf16le(const wstring& ws)
{
    typedef utf16_detail::utf16_impl<sizeof(wchar_t)> impl_type;
    return impl_type::to_utf16<hamigaki::little>(ws);
}

} } // End namespaces charset, hamigaki.

#endif // HAMIGAKI_CHARSET_UTF16_HPP
