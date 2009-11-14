// environment.hpp: an utility for Windows environment variables

// Copyright Takeshi Mouri 2006-2009.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_WINDOWS_ENVIRONMENT_HPP
#define HAMIGAKI_DETAIL_WINDOWS_ENVIRONMENT_HPP

#include <boost/shared_ptr.hpp>
#include <cstring>
#include <map>
#include <string>
#include <windows.h>

namespace hamigaki { namespace detail { namespace windows {

struct iless
{
    bool operator()(const std::string& lhs, const std::string& rhs) const
    {
        return ::CompareStringA(
            LOCALE_USER_DEFAULT, NORM_IGNORECASE,
            lhs.c_str(), static_cast<int>(lhs.size()),
            rhs.c_str(), static_cast<int>(rhs.size())
        ) == CSTR_LESS_THAN;
    }
};

typedef std::map<std::string,std::string,iless> environment_type;

inline void get_environment_variables(environment_type& table)
{
    // Note: GetEnvironmentStringsA is macro for GetEnvironmentStrings
    boost::shared_ptr<void> env(
        ::GetEnvironmentStrings(),
        ::FreeEnvironmentStringsA
    );

    for (const char* s = static_cast<char*>(env.get()); *s; )
    {
        if (const char* delim = std::strchr(s, '='))
        {
            if (delim != s)
                table[std::string(s, delim)] = delim+1;
        }

        s += std::strlen(s) + 1;
    }
}

} } } // End namespaces windows, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_WINDOWS_ENVIRONMENT_HPP
