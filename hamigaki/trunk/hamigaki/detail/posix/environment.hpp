// environment.hpp: an utility for POSIX environment variables

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef HAMIGAKI_DETAIL_POSIX_ENVIRONMENT_HPP
#define HAMIGAKI_DETAIL_POSIX_ENVIRONMENT_HPP

#include <cstring>
#include <map>
#include <string>
#include <unistd.h>

#if defined(__APPLE__) && defined(__DYNAMIC__)
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

namespace hamigaki { namespace detail { namespace posix {

typedef std::map<std::string,std::string> environment_type;

inline void get_environment_variables(environment_type& table)
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
