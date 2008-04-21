// iso_string.hpp: string utility functions for ISO 9660

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_ISO_STRING_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_ISO_STRING_HPP

#include <hamigaki/archivers/detail/ucs2.hpp>

#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
    #include <hamigaki/charset/code_page.hpp>
    #include <hamigaki/charset/utf16.hpp>
#endif

namespace hamigaki { namespace archivers { namespace detail {

inline std::string to_iso9660_string(const std::string& s)
{
    return s;
}

inline std::string to_joliet_string(const std::string& s)
{
    return detail::narrow_to_ucs2be(s);
}
#if !defined(BOOST_FILESYSTEM_NARROW_ONLY)
inline std::string to_iso9660_string(const std::wstring& s)
{
    return charset::to_code_page(s,0,"_");
}

inline std::string to_joliet_string(const std::wstring& s)
{
    return charset::to_utf16be(s);
}
#endif // !defined(BOOST_FILESYSTEM_NARROW_ONLY)

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_ISO_STRING_HPP
