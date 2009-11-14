// code_page.hpp: utility for Windows code pages

// Copyright Takeshi Mouri 2008, 2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/charset for library home page.

#ifndef HAMIGAKI_CHARSET_CODE_PAGE_HPP
#define HAMIGAKI_CHARSET_CODE_PAGE_HPP

#include <hamigaki/charset/wstring.hpp>
#include <hamigaki/dec_format.hpp>
#include <boost/detail/endian.hpp>
#include <boost/scoped_array.hpp>
#include <cstring>
#include <stdexcept>

#if defined(BOOST_WINDOWS)
    #include <hamigaki/charset/detail/winnls.hpp>
#else
    #include <hamigaki/charset/detail/iconv.hpp>
    #include <errno.h>

    #if defined(__CYGWIN__)
        extern "C" __declspec(dllimport) unsigned __stdcall GetACP();
    #else
        #include <langinfo.h>
    #endif
#endif

namespace hamigaki { namespace charset {

#if defined(BOOST_WINDOWS)
inline std::string to_code_page(
    const wstring& ws, unsigned cp, const char* def_char, bool* used_def_char)
{
    if (ws.empty())
    {
        if (used_def_char)
            *used_def_char = false;
        return std::string();
    }

    int len = ::WideCharToMultiByte(
        cp, 0, ws.c_str(), static_cast<int>(ws.size()), 0, 0, def_char, 0);
    if (len == 0)
        throw std::runtime_error("failed WideCharToMultiByte()");

    boost::scoped_array<char> buf(new char[len]);

    int used_def = 0;
    len = ::WideCharToMultiByte(
        cp, 0, ws.c_str(), static_cast<int>(ws.size()),
        buf.get(), len, def_char,
        used_def_char ? &used_def : static_cast<int*>(0));
    if (len == 0)
        throw std::runtime_error("failed WideCharToMultiByte()");

    if (used_def_char)
        *used_def_char = (used_def != 0);

    return std::string(buf.get(), buf.get()+len);
}

inline wstring from_code_page(const std::string& s, unsigned cp)
{
    if (s.empty())
        return wstring();

    int len = ::MultiByteToWideChar(
        cp, 0, s.c_str(), static_cast<int>(s.size()), 0, 0);
    if (len == 0)
        throw std::runtime_error("failed MultiByteToWideChar()");

    boost::scoped_array<wchar_t> buf(new wchar_t[len]);

    len = ::MultiByteToWideChar(
        cp, 0, s.c_str(), static_cast<int>(s.size()), buf.get(), len);
    if (len == 0)
        throw std::runtime_error("failed MultiByteToWideChar()");

    return wstring(buf.get(), buf.get()+len);
}
#else // !defined(BOOST_WINDOWS)
namespace cp_detail
{

inline std::string make_cp_name(unsigned cp)
{
    if (cp == 0)
    {
#if defined(__CYGWIN__)
        cp = ::GetACP();
#elif defined(__GLIBC__) || defined(_LIBICONV_VERSION)
        return "";
#else
        // FIXME: MT unsafe
        return ::nl_langinfo(CODESET);
#endif
    }

    switch (cp)
    {
        case 20932:
            return "EUC-JP";
        case 50220:
            return "ISO-2022-JP";
        case 65000:
            return "UTF-7";
        case 65001:
            return "UTF-8";
        default:
            return "CP" + hamigaki::to_dec<char>(cp);
    }
}

template<std::size_t CharSize>
struct wide_code_page_tarits;

template<>
struct wide_code_page_tarits<2>
{
    static const char* name()
    {
#if defined(BOOST_LITTLE_ENDIAN)
        return "UTF-16LE";
#else
        return "UTF-16BE";
#endif
    }
};

template<>
struct wide_code_page_tarits<4>
{
    static const char* name()
    {
#if defined(BOOST_LITTLE_ENDIAN)
        return "UTF-32LE";
#else
        return "UTF-32BE";
#endif
    }
};

template<std::size_t CharSize>
struct to_code_page_impl;

template<>
struct to_code_page_impl<2>
{
    static std::string convert(
        const wstring& ws,
        unsigned cp, const char* def_char, bool* used_def_char)
    {
        if (!def_char)
            def_char = "?";

        const std::string& narrow_cp = make_cp_name(cp);
        const char* wide_cp = wide_code_page_tarits<sizeof(wchar_t)>::name();

        detail::iconv_wrapper cv(narrow_cp.c_str(), wide_cp);

        char src_buf[4];
        char dst_buf[64];
        std::string s;

        std::size_t i = 0;
        while (i < ws.size())
        {
            bool surrogate = (static_cast<unsigned>(ws[i]) & 0xF800) == 0xD800;

            std::memcpy(src_buf, &ws[i], sizeof(wchar_t));
            char* src = src_buf;
            std::size_t src_size = sizeof(wchar_t);

            if (surrogate)
            {
                std::memcpy(src_buf+sizeof(wchar_t), &ws[i+1], sizeof(wchar_t));
                src_size += sizeof(wchar_t);
            }

            char* dst = dst_buf;
            std::size_t dst_size = sizeof(dst_buf);

            std::size_t res = cv.convert(src, src_size, dst, dst_size);
            if (res != 0)
            {
                cv.flush(dst, dst_size);
                s.append(&dst_buf[0], dst);
                cv.reset();

                s.append(def_char);
                if (used_def_char)
                    *used_def_char = true;

                ++i;
            }
            else
            {
                s.append(&dst_buf[0], dst);

                if (surrogate)
                    i += 2;
                else
                    ++i;
            }
        }

        char* dst = dst_buf;
        std::size_t dst_size = sizeof(dst_buf);
        cv.flush(dst, dst_size);
        s.append(&dst_buf[0], dst);

        return s;
    }
};

template<>
struct to_code_page_impl<4>
{
    static std::string convert(
        const wstring& ws,
        unsigned cp, const char* def_char, bool* used_def_char)
    {
        if (!def_char)
            def_char = "?";

        const std::string& narrow_cp = make_cp_name(cp);
        const char* wide_cp = wide_code_page_tarits<sizeof(wchar_t)>::name();

        detail::iconv_wrapper cv(narrow_cp.c_str(), wide_cp);

        char src_buf[4];
        char dst_buf[64];
        std::string s;

        for (std::size_t i = 0; i < ws.size(); ++i)
        {
            std::memcpy(src_buf, &ws[i], sizeof(wchar_t));
            char* src = src_buf;
            std::size_t src_size = sizeof(wchar_t);

            char* dst = dst_buf;
            std::size_t dst_size = sizeof(dst_buf);

            std::size_t res = cv.convert(src, src_size, dst, dst_size);
            if (res != 0)
            {
                cv.flush(dst, dst_size);
                s.append(&dst_buf[0], dst);
                cv.reset();

                s.append(def_char);
                if (used_def_char)
                    *used_def_char = true;
            }
            else
                s.append(&dst_buf[0], dst);
        }

        char* dst = dst_buf;
        std::size_t dst_size = sizeof(dst_buf);
        cv.flush(dst, dst_size);
        s.append(&dst_buf[0], dst);

        return s;
    }
};

} // namespace cp_detail

inline std::string to_code_page(
    const wstring& ws, unsigned cp, const char* def_char, bool* used_def_char)
{
    typedef cp_detail::to_code_page_impl<sizeof(wchar_t)> impl_type;
    return impl_type::convert(ws, cp, def_char, used_def_char);
}

inline wstring from_code_page(const std::string& s, unsigned cp)
{
    if (s.empty())
        return wstring();

    const std::string& narrow_cp = cp_detail::make_cp_name(cp);
    const char* wide_cp =
        cp_detail::wide_code_page_tarits<sizeof(wchar_t)>::name();

    boost::scoped_array<char> src_buf(new char[s.size()]);
    s.copy(src_buf.get(), s.size());

    char* src = src_buf.get();
    std::size_t src_size = s.size();

    detail::iconv_wrapper cv(wide_cp, narrow_cp.c_str());

    char dst_buf[64];
    wchar_t tmp[64];
    wstring ws;
    while (src_size != 0)
    {
        char* dst = dst_buf;
        std::size_t dst_size = sizeof(dst_buf);

        std::size_t res = cv.convert(src, src_size, dst, dst_size);
        if (res == detail::iconv_wrapper::error)
        {
            if (errno != E2BIG)
                throw std::runtime_error("failed iconv()");
        }

        std::size_t len = dst-dst_buf;
        std::memcpy(tmp, dst_buf, len);
        ws.append(&tmp[0], &tmp[0]+len/sizeof(wchar_t));
    }
    return ws;
}
#endif // !defined(BOOST_WINDOWS)

inline std::string
to_code_page(const wstring& ws, unsigned cp, const char* def_char)
{
    return hamigaki::charset::to_code_page(ws, cp, def_char, 0);
}

inline std::string to_code_page(const wstring& ws, unsigned cp)
{
    return hamigaki::charset::to_code_page(ws, cp, 0, 0);
}

} } // End namespaces charset, hamigaki.

#endif // HAMIGAKI_CHARSET_WSTRING_HPP
