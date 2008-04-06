// file_status.cpp: the file status operations

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#define HAMIGAKI_FILESYSTEM_SOURCE
#define NOMINMAX

#if !defined(BOOST_ALL_NO_LIB)
    #define BOOST_ALL_NO_LIB
#endif

#if !defined(_WIN32_WINNT)
    #define _WIN32_WINNT 0x0500
#endif

#include <boost/config.hpp>

#include "hamigaki/filesystem/operations.hpp"

#if defined(BOOST_WINDOWS_API)
    #include "windows/file_status.ipp"
#else
    #include "posix/file_status.ipp"
#endif
