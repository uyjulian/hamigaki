//  tar_ex_header.hpp: extended tar header

//  Copyright Takeshi Mouri 2006.
//  Use, modification, and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://hamigaki.sourceforge.jp/libs/iostreams for library home page.

#ifndef HAMIGAKI_ARCHIVERS_DETAIL_TAR_EX_HEADER_HPP
#define HAMIGAKI_ARCHIVERS_DETAIL_TAR_EX_HEADER_HPP

#include <hamigaki/filesystem/timestamp.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/cstdint.hpp>
#include <boost/optional.hpp>
#include <string>

namespace hamigaki { namespace archivers { namespace detail {

struct tar_ex_header
{
    boost::filesystem::path path;
    boost::optional<boost::intmax_t> uid;
    boost::optional<boost::intmax_t> gid;
    boost::optional<boost::uintmax_t> file_size;
    boost::optional<filesystem::timestamp> modified_time;
    boost::optional<filesystem::timestamp> access_time;
    boost::optional<filesystem::timestamp> change_time;
    boost::filesystem::path link_path;
    std::string user_name;
    std::string group_name;
    std::string comment;
};

} } } // End namespaces detail, archivers, hamigaki.

#endif // HAMIGAKI_ARCHIVERS_DETAIL_TAR_EX_HEADER_HPP
