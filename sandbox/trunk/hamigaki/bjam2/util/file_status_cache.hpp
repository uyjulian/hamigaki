// file_status_cache.hpp: cache for file status

// Copyright Takeshi Mouri 2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://hamigaki.sourceforge.jp/libs/bjam for library home page.

#ifndef HAMIGAKI_BJAM2_UTIL_FILE_STATUS_HPP
#define HAMIGAKI_BJAM2_UTIL_FILE_STATUS_HPP

#include <hamigaki/bjam2/bjam_config.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <map>
#include <string>
#include <vector>

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_PREFIX
#endif

#ifdef BOOST_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4251)
#endif

namespace hamigaki { namespace bjam2 {

struct file_info
{
    boost::filesystem::file_status status;
    boost::optional<std::vector<std::string> > entries;
};

class HAMIGAKI_BJAM2_DECL file_status_cache
{
public:
    file_status_cache();
    void working_directory(const std::string& dir);
    const boost::filesystem::file_status& status(const std::string& filename);

    const std::vector<std::string>&
    directory_entries(const std::string& dirname);

private:
    typedef std::map<std::string,file_info> table_type;
    table_type table_;
    boost::filesystem::path work_;
};

} } // End namespaces bjam2, hamigaki.

#ifdef BOOST_MSVC
    #pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
    #include BOOST_ABI_SUFFIX
#endif

#endif // HAMIGAKI_BJAM2_UTIL_FILE_STATUS_HPP
