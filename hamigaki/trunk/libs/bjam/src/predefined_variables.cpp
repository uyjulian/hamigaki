// predefined_variables.cpp: bjam pre-defined variables

// Copyright Takeshi Mouri 2007.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#define HAMIGAKI_BJAM_SOURCE
#include <hamigaki/bjam/bjam_context.hpp>
#include <hamigaki/bjam/bjam_version.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/none.hpp>
#include <ctime>

#if defined(__unix__)
    #include <sys/utsname.h>
#endif

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    #include <time.h>
#endif


#if defined(BOOST_WINDOWS)
    #define HAMIGAKI_BJAM_OSMAJOR "NT"
    #define HAMIGAKI_BJAM_OSMINOR "NT"
#else
    #define HAMIGAKI_BJAM_OSMAJOR "UNIX"
    #if defined(__CYGWIN__)
        #define HAMIGAKI_BJAM_OSMINOR "CYGWIN"
    #elif defined(linux) || defined(__linux) || defined(__linux__)
        #define HAMIGAKI_BJAM_OSMINOR "LINUX"
    #else
        #define HAMIGAKI_BJAM_OSMINOR "UNKNOWN"
    #endif
#endif

#if defined(_M_IX86) || defined(__i386__)
    #define HAMIGAKI_BJAM_OSPLAT "X86"
#elif defined( __ia64__ ) || defined( __IA64__ )
    #define HAMIGAKI_BJAM_OSPLAT "IA64"
#else
    #define HAMIGAKI_BJAM_OSPLAT ""
#endif

namespace hamigaki { namespace bjam {

namespace
{

std::string convert_time(std::time_t t)
{
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    char buf[26];
    return ::ctime_r(&t, buf);
#else
    return std::ctime(&t);
#endif
}

} // namespace

HAMIGAKI_BJAM_DECL void set_predefined_variables(context& ctx)
{
    module& m = ctx.get_module(boost::none);

    std::string jamdate = convert_time(std::time(0));
    jamdate.resize(24);
    m.variables.set_values("JAMDATE", string_list(jamdate));

    m.variables.set_values(
        "JAM_VERSION",
        boost::assign::list_of
            (HAMIGAKI_BJAM_VERSION_MAJOR_SYM)
            (HAMIGAKI_BJAM_VERSION_MINOR_SYM)
            (HAMIGAKI_BJAM_VERSION_PATCH_SYM)
    );

#if defined(__unix__)
    ::utsname buf;
    if (uname(&buf) == 0)
    {
        m.variables.set_values(
            "JAMUNAME",
            boost::assign::list_of
                (buf.sysname)
                (buf.nodename)
                (buf.release)
                (buf.version)
                (buf.machine)
        );
    }
#endif

    m.variables.set_values(HAMIGAKI_BJAM_OSMAJOR, string_list("true"));
    m.variables.set_values("OS", string_list(HAMIGAKI_BJAM_OSMINOR));
    m.variables.set_values("OSPLAT", string_list(HAMIGAKI_BJAM_OSPLAT));
    m.variables.set_values("JAMVERSION", string_list(HAMIGAKI_BJAM_JAMVERSION));
}

} } // End namespaces bjam, hamigaki.
