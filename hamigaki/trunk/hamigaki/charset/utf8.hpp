// utf8.hpp: utility for UTF-8

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#ifndef HAMIGAKI_CHARSET_UTF8_HPP
#define HAMIGAKI_CHARSET_UTF8_HPP

#include <hamigaki/charset/exception.hpp>
#include <hamigaki/charset/wstring.hpp>

namespace hamigaki { namespace charset {

namespace utf8_detail
{

inline char to_char(boost::uint32_t n)
{
    return static_cast<char>(static_cast<unsigned char>(n));
}

inline boost::uint32_t to_ui32(char c)
{
    return static_cast<boost::uint32_t>(static_cast<unsigned char>(c));
}

inline char* ui32_to_utf8(char* s, boost::uint32_t wc)
{
    if ((wc & 0xFFFFFF80) == 0)
        *(s++) = to_char(wc & 0x7F);
    else if ((wc & 0xFFFFF800) == 0)
    {
        *(s++) = to_char(0xC0 | ((wc >> 6) & 0x1F));
        *(s++) = to_char(0x80 | ((wc     ) & 0x3F));
    }
    else if ((wc & 0xFFFF0000) == 0)
    {
        *(s++) = to_char(0xE0 | ((wc >> 12) & 0x0F));
        *(s++) = to_char(0x80 | ((wc >>  6) & 0x3F));
        *(s++) = to_char(0x80 | ((wc      ) & 0x3F));
    }
    else if (wc <= 0x10FFFF)
    {
        *(s++) = to_char(0xF0 | ((wc >> 18) & 0x07));
        *(s++) = to_char(0x80 | ((wc >> 12) & 0x3F));
        *(s++) = to_char(0x80 | ((wc >>  6) & 0x3F));
        *(s++) = to_char(0x80 | ((wc      ) & 0x3F));
    }
    else
        return 0;

    return s;
}

inline bool utf8_to_ui32(boost::uint32_t& wc, const char*& s, std::size_t n)
{
    boost::uint32_t uc = to_ui32(*s);

    boost::uint32_t shift = 0;
    if ((uc & 0x80) == 0)
        wc = uc;
    else if ((uc & 0x40) == 0)
        return false;
    else if ((uc & 0x20) == 0)
    {
        if (n < 2)
            return false;

        wc = (uc & 0x1F) << 6;
        shift = 6;
    }
    else if ((uc & 0x10) == 0)
    {
        if (n < 3)
            return false;

        wc = (uc & 0x0F) << 12;
        shift = 12;
    }
    else if ((uc & 0x08) == 0)
    {
        if (n < 4)
            return false;

        wc = (uc & 0x07) << 18;
        shift = 18;
    }
    else if ((uc & 0x04) == 0)
    {
        if (n < 5)
            return false;

        wc = (uc & 0x03) << 24;
        shift = 24;
    }
    else if ((uc & 0x02) == 0)
    {
        if (n < 6)
            return false;

        wc = (uc & 0x01) << 30;
        shift = 30;
    }
    else
        return false;

    ++s;

    while (shift != 0)
    {
        shift -= 6;
        uc = to_ui32(*s);
        if ((uc & 0xC0) != 0x80)
            return false;
        ++s;
        wc |= (uc & 0x3F) << shift;
    }

    return true;
}

template<std::size_t CharSize>
struct utf8_impl;

template<>
struct utf8_impl<2>
{
    static std::string to_utf8(const wstring& ws)
    {
        std::size_t n = ws.size();

        std::string s;

        char buf[4];
        boost::uint16_t surrogate = 0;
        for (std::size_t i = 0; i < n; ++i)
        {
            boost::uint16_t u = static_cast<boost::uint16_t>(ws[i]);
            boost::uint32_t wc = u;

            if (surrogate != 0)
            {
                if ((u & 0xFC00) != 0xDC00)
                    throw invalid_surrogate_pair(surrogate, u);

                wc &= 0x3FF;
                wc |= (surrogate & 0x3FF) << 10;
                wc += 0x10000;
                surrogate = 0;
            }
            else if ((u & 0xFC00) == 0xDC00)
                throw missing_high_surrogate(u);
            else if ((u & 0xF800) == 0xD800)
            {
                surrogate = u;
                continue;
            }

            char* end = ui32_to_utf8(buf, wc);
            if (end == 0)
                throw invalid_ucs4(wc);
            s.append(buf, end);
        }

        if (surrogate != 0)
            throw missing_low_surrogate(surrogate);

        return s;
    }

    static wstring from_utf8(const std::string& s)
    {
        wstring ws;
        const char* first = s.c_str();
        const char* last = first + s.size();
        while (first != last)
        {
            boost::uint32_t wc;
            if (!utf8_to_ui32(wc, first, last-first))
                throw invalid_utf8(*first);

            if ((wc & 0xFFFF0000) == 0)
                ws.push_back(static_cast<wchar_t>(wc));
            else if (wc <= 0x10FFFF)
            {
                wc -= 0x10000;
                ws.push_back(static_cast<wchar_t>(0xD800 | ((wc>>10) & 0x3FF)));
                ws.push_back(static_cast<wchar_t>(0xDC00 | ((wc    ) & 0x3FF)));
            }
            else
                throw invalid_ucs4(wc);
        }

        return ws;
    }
};

template<>
struct utf8_impl<4>
{
    static std::string to_utf8(const wstring& ws)
    {
        std::size_t n = ws.size();

        std::string s;

        char buf[4];
        for (std::size_t i = 0; i < n; ++i)
        {
            boost::uint32_t wc = static_cast<boost::uint32_t>(ws[i]);
            char* end = ui32_to_utf8(buf, wc);
            if (end == 0)
                throw invalid_ucs4(wc);
            s.append(buf, end);
        }
        return s;
    }

    static wstring from_utf8(const std::string& s)
    {
        wstring ws;
        const char* first = s.c_str();
        const char* last = first + s.size();
        while (first != last)
        {
            boost::uint32_t wc;
            if (!utf8_to_ui32(wc, first, last-first))
                throw invalid_utf8(*first);

            if (wc > 0x10FFFF)
                throw invalid_ucs4(wc);

            ws.push_back(static_cast<wchar_t>(wc));
        }

        return ws;
    }
};

} // namespace utf8_detail

inline std::string to_utf8(const wstring& ws)
{
    typedef utf8_detail::utf8_impl<sizeof(wchar_t)> impl_type;
    return impl_type::to_utf8(ws);
}

inline wstring from_utf8(const std::string& s)
{
    typedef utf8_detail::utf8_impl<sizeof(wchar_t)> impl_type;
    return impl_type::from_utf8(s);
}

} } // End namespaces charset, hamigaki.

#endif // HAMIGAKI_CHARSET_UTF8_HPP
