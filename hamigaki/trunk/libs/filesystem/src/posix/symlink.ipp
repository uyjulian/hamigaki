// symlink.ipp: the symbolic link operations for POSIX

// Copyright Takeshi Mouri 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/filesystem for library home page.

#include <hamigaki/filesystem/detail/config.hpp>
#include <boost/scoped_array.hpp>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

namespace hamigaki { namespace filesystem { namespace detail {

HAMIGAKI_FILESYSTEM_DECL error_code
symlink_target_api(const std::string& ph, std::string& target)
{
    struct stat st;
    if (::lstat(ph.c_str(), &st) == -1)
        return make_error_code(errno);

    if (!S_ISLNK(st.st_mode))
        return make_error_code(EINVAL);

    boost::scoped_array<char> buf(new char[st.st_size]);
    std::streamsize len = ::readlink(ph.c_str(), buf.get(), st.st_size);
    if (len == -1)
        return make_error_code(errno);

    target.assign(buf.get(), buf.get()+len);
    return error_code();
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_hard_link_api(
    const std::string& to_ph, const std::string& from_ph)
{
    if (::link(to_ph.c_str(), from_ph.c_str()) == -1)
        return make_error_code(errno);
    else
        return error_code();
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_file_symlink_api(
    const std::string& to_ph, const std::string& from_ph)
{
    return detail::create_symlink_api(to_ph, from_ph);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_directory_symlink_api(
    const std::string& to_ph, const std::string& from_ph)
{
    return detail::create_symlink_api(to_ph, from_ph);
}

HAMIGAKI_FILESYSTEM_DECL error_code
create_symlink_api(const std::string& to_ph, const std::string& from_ph)
{
    if (::symlink(to_ph.c_str(), from_ph.c_str()) == -1)
        return make_error_code(errno);
    else
        return error_code();
}

} } } // End namespaces detail, filesystem, hamigaki.
