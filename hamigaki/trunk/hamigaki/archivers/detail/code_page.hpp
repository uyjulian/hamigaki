// code_page.hpp: utility functions for codd pages

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/archivers for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_CODE_PAGE_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_CODE_PAGE_HPP

#include <hamigaki/charset/code_page.hpp>

namespace hamigaki { namespace archivers { namespace detail {

class to_code_page
{
public:
    to_code_page() : cp_(0), def_char_(0), used_def_char_(0)
    {
    }

    explicit to_code_page(unsigned cp)
        : cp_(cp), def_char_(0), used_def_char_(0)
    {
    }

    to_code_page(unsigned cp, const char* def_char)
        : cp_(cp), def_char_(def_char), used_def_char_(0)
    {
    }

    to_code_page(unsigned cp, const char* def_char, bool* used_def_char)
        : cp_(cp), def_char_(def_char), used_def_char_(used_def_char)
    {
    }

    std::string operator()(const std::basic_string<wchar_t>& ws) const
    {
        return hamigaki::charset::to_code_page(
            ws, cp_, def_char_, used_def_char_);
    }

private:
    const unsigned cp_;
    const char* def_char_;
    bool* used_def_char_;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_CODE_PAGE_HPP
