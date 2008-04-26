// tar_ex_header.hpp: extended tar header

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_TAR_EX_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_TAR_EX_HEADER_HPP

#include <hamigaki/filesystem/timestamp.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <string>

namespace hamigaki { namespace archivers { namespace detail {

template<class Path>
struct tar_ex_header
{
    typedef Path path_type;
    typedef typename Path::string_type string_type;

    Path path;
    boost::optional<boost::intmax_t> uid;
    boost::optional<boost::intmax_t> gid;
    boost::optional<boost::uintmax_t> file_size;
    boost::optional<filesystem::timestamp> modified_time;
    boost::optional<filesystem::timestamp> access_time;
    boost::optional<filesystem::timestamp> change_time;
    Path link_path;
    string_type user_name;
    string_type group_name;
    string_type comment;
    string_type charset;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_TAR_EX_HEADER_HPP
