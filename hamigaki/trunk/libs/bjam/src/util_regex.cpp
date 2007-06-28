// util_regex.cpp: an utility for regular expression

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/util/regex.hpp>

namespace hamigaki { namespace bjam {

HAMIGAKI_BJAM_DECL std::string convert_regex(const std::string& s)
{
    std::string result;

    std::size_t i = 0;
    std::size_t size = s.size();
    while (i < size)
    {
        char c = s[i++];
        if (i == size)
            result += c;
        else if (c == '\\')
        {
            c = s[i];
            if ((c == '<') || (c == '>') || (c == '(') || (c == ')') ||
                (c == '[') || (c == ']') || (c == '*') || (c == '+') ||
                (c == '.') || (c == '\\') )
            {
                ++i;
                result += '\\';
                result += c;
            }
            else
                result += "\\\\";
        }
        else if (c == '[')
        {
            result += '[';
            while (i < size)
            {
                c = s[i++];
                if (c == '\\')
                    result += "\\\\";
                else
                    result += c;
                if (c == ']')
                    break;
            }
        }
        else if (c == '|')
        {
            // FIXME: ad-hoc workaround
            // "|)" -> ")?"
            c = s[i];
            if (c == ')')
            {
                ++i;
                result += ")?";
            }
            else
                result += '|';
        }
        else
            result += c;
    }

    return result;
}

} } // End namespaces bjam, hamigaki.
