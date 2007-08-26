// environment.hpp: an utility for POSIX environment variables

// Copyright Takeshi Mouri 2006, 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_POSIX_ENVIRONMENT_HPP
#define HAMIGAKI_DETAIL_POSIX_ENVIRONMENT_HPP

#include <cstring>
#include <map>
#include <string>
#include <unistd.h>

#if defined(__APPLE__)
    #if defined(__DYNAMIC__)
        #include <crt_externs.h>
        #if !defined(environ)
            #define environ (*_NSGetEnviron())
        #endif
    #else
        extern "C"
        {
            extern char** environ;
        }
    #endif
#endif

namespace hamigaki { namespace detail { namespace posix {

inline void get_environment_variables(std::map<std::string,std::string>& table)
{
    for (char** p = environ; *p; ++p)
    {
        const char* s = *p;
        if (const char* delim = std::strchr(s, '='))
            table[std::string(s, delim-s)].assign(delim+1);
    }
}

} } } // End namespaces posix, detail, hamigaki.

#endif // HAMIGAKI_DETAIL_POSIX_ENVIRONMENT_HPP
